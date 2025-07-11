#include <graphics.h>
#include <deque>
#include <time.h>
#include <mmsystem.h>
#include <vector>
#pragma comment(lib, "winmm.lib")

// 游戏常量定义
const int BLOCK_SIZE = 20;    // 每个蛇身块的像素大小
const int WIDTH = 30;         // 游戏区域宽度（单位：块）
const int HEIGHT = 20;        // 游戏区域高度（单位：块）
const int NORMAL_SPEED = 150; // 正常游戏速度（毫秒）
const int FAST_SPEED = 75;    // 加速后游戏速度（毫秒）

// 方向枚举
enum Direction { UP, DOWN, LEFT, RIGHT };

// 蛇类
class Snake {
public:
    std::deque<POINT> body;   // 使用双端队列存储蛇身坐标
    Direction dir;            // 当前移动方向
    int score;                // 游戏得分

    Snake() {
        // 初始化蛇身（3节）
        body.push_back({ WIDTH / 2, HEIGHT / 2 });
        body.push_back({ WIDTH / 2 - 1, HEIGHT / 2 });
        body.push_back({ WIDTH / 2 - 2, HEIGHT / 2 });
        dir = RIGHT;
        score = 0;
    }

    // 移动蛇身
    void move() {
        POINT head = body.front();
        POINT newHead = head;

        // 根据方向计算新头部位置
        switch (dir) {
        case UP:    newHead.y--; break;
        case DOWN:  newHead.y++; break;
        case LEFT:  newHead.x--; break;
        case RIGHT: newHead.x++; break;
        }

        body.push_front(newHead);  // 添加新头部
        body.pop_back();           // 移除尾部（除非吃到食物）
    }

    // 增长蛇身
    void grow() {
        body.push_back(body.back());
        score += 10;
    }

    // 碰撞检测
    bool checkCollision() {
        POINT head = body.front();

        // 边界检测
        if (head.x < 0 || head.x >= WIDTH || head.y < 0 || head.y >= HEIGHT)
            return true;

        // 自碰检测
        for (auto it = ++body.begin(); it != body.end(); ++it) {
            if (head.x == it->x && head.y == it->y)
                return true;
        }

        return false;
    }
};

// 食物类
class Food {
public:
    POINT pos;

    Food(const Snake& snake, const std::vector<POINT>& obstacles) {
        generate(snake, obstacles);
    }

    // 生成新食物（避开蛇身和障碍物）
    void generate(const Snake& snake, const std::vector<POINT>& obstacles) {
        bool onSnakeOrObstacle;
        do {
            onSnakeOrObstacle = false;
            pos.x = rand() % WIDTH;
            pos.y = rand() % HEIGHT;

            // 检查是否生成在蛇身上或障碍物上
            for (const auto& segment : snake.body) {
                if (pos.x == segment.x && pos.y == segment.y) {
                    onSnakeOrObstacle = true;
                    break;
                }
            }
            for (const auto& obstacle : obstacles) {
                if (pos.x == obstacle.x && pos.y == obstacle.y) {
                    onSnakeOrObstacle = true;
                    break;
                }
            }
        } while (onSnakeOrObstacle);
    }
};

// 障碍物类
class Obstacle {
public:
    std::vector<POINT> obstacles;

    // 生成新障碍物（避开蛇身和食物）
    void generate(const Snake& snake, const Food& food, int count) {
        for (int i = 0; i < count; i++) {
            bool onSnakeOrFood;
            POINT newObstacle;
            do {
                onSnakeOrFood = false;
                newObstacle.x = rand() % WIDTH;
                newObstacle.y = rand() % HEIGHT;

                // 检查是否生成在蛇身上或食物上
                for (const auto& segment : snake.body) {
                    if (newObstacle.x == segment.x && newObstacle.y == segment.y) {
                        onSnakeOrFood = true;
                        break;
                    }
                }
                if (newObstacle.x == food.pos.x && newObstacle.y == food.pos.y) {
                    onSnakeOrFood = true;
                }
            } while (onSnakeOrFood);

            obstacles.push_back(newObstacle);
        }
    }
};

// 绘制游戏界面
void drawGame(const Snake& snake, const Food& food, const Obstacle& obstacle) {
    cleardevice();  // 清空屏幕

    // 绘制网格线
    setlinecolor(LIGHTGRAY);
    for (int i = 0; i <= WIDTH; i++)
        line(i * BLOCK_SIZE, 0, i * BLOCK_SIZE, HEIGHT * BLOCK_SIZE);
    for (int i = 0; i <= HEIGHT; i++)
        line(0, i * BLOCK_SIZE, WIDTH * BLOCK_SIZE, i * BLOCK_SIZE);

    // 绘制蛇身（绿色渐变）
    int colorStep = 255 / snake.body.size();
    for (int i = 0; i < snake.body.size(); i++) {
        setfillcolor(RGB(0, 200 - i * colorStep, 0));
        fillrectangle(
            snake.body[i].x * BLOCK_SIZE + 1,
            snake.body[i].y * BLOCK_SIZE + 1,
            (snake.body[i].x + 1) * BLOCK_SIZE - 1,
            (snake.body[i].y + 1) * BLOCK_SIZE - 1
        );
    }

    // 绘制食物（红色圆形）
    setfillcolor(RED);
    fillcircle(
        food.pos.x * BLOCK_SIZE + BLOCK_SIZE / 2,
        food.pos.y * BLOCK_SIZE + BLOCK_SIZE / 2,
        BLOCK_SIZE / 2 - 2
    );

    // 绘制障碍物（灰色方块）
    setfillcolor(BLACK);
    for (const auto& obs : obstacle.obstacles) {
        fillrectangle(
            obs.x * BLOCK_SIZE + 1,
            obs.y * BLOCK_SIZE + 1,
            (obs.x + 1) * BLOCK_SIZE - 1,
            (obs.y + 1) * BLOCK_SIZE - 1
        );
    }

    // 显示分数
    settextcolor(BLACK);
    settextstyle(20, 0, _T("Arial"));
    TCHAR scoreText[20];
    sprintf_s(scoreText, _T("Score: %d"), snake.score);
    outtextxy(10, 10, scoreText);
}

// 处理图形窗口中的键盘和鼠标输入
void processInput(Snake& snake, bool& paused, int& speed) {
    ExMessage msg;
    while (peekmessage(&msg)) {
        if (msg.message == WM_KEYDOWN) {
            switch (msg.vkcode) {
            case 'w': case 'W':  // 上
                if (snake.dir != DOWN) snake.dir = UP;
                break;
            case 's': case 'S':  // 下
                if (snake.dir != UP) snake.dir = DOWN;
                break;
            case 'a': case 'A':  // 左
                if (snake.dir != RIGHT) snake.dir = LEFT;
                break;
            case 'd': case 'D':  // 右
                if (snake.dir != LEFT) snake.dir = RIGHT;
                break;
            case 'p': case 'P':  // P键暂停/继续
                paused = !paused;
                break;
            case 27:  // ESC键退出
                exit(0);
            }
        }
        else if (msg.message == WM_LBUTTONDOWN) {  // 鼠标左键按下加速
            speed = FAST_SPEED;
        }
        else if (msg.message == WM_LBUTTONUP) {    // 鼠标左键松开恢复正常速度
            speed = NORMAL_SPEED;
        }
    }
}

// 显示开始界面
void showStartScreen() {
    cleardevice();  // 清空屏幕
    setbkcolor(WHITE);  // 设置背景色
    settextcolor(BLACK);

    IMAGE background;
    loadimage(&background, _T("back.jpg"));  // 加载背景图片
    putimage(0, 0, &background);  // 绘制背景图片

    settextstyle(40, 0, _T("Arial"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 - 50, _T("贪吃蛇游戏"));
    settextstyle(20, 0, _T("Arial"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 150, HEIGHT * BLOCK_SIZE / 2 + 50, _T("按任意键开始游戏"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 + 100, _T("方向键'w''a''s''d'控制蛇的上左下右移动"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 + 150, _T("ESC键退出"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 + 200, _T("按住鼠标左键加速"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 + 250, _T("P键暂停/继续"));

    // 等待用户按下任意键开始游戏
    while (true) {
        ExMessage msg;
        if (peekmessage(&msg)) {
            if (msg.message == WM_KEYDOWN) {
                return;  // 开始游戏
            }
        }
    }
}

int main() {
    initgraph(WIDTH * BLOCK_SIZE, HEIGHT * BLOCK_SIZE);  // 创建图形窗口
    setbkcolor(WHITE);                                   // 设置背景色
    srand((unsigned)time(NULL));                         // 初始化随机种子

    showStartScreen();                                   // 显示开始界面

    // 播放背景音乐
    mciSendString(_T("open BGM.mp3 alias bgm"), NULL, 0, NULL);
    mciSendString(_T("play bgm repeat"), NULL, 0, NULL);

    Snake snake;
    Food food(snake, {});
    Obstacle obstacle;
    int foodCounter = 0;  // 记录吃掉的食物数量
    bool paused = false;   // 游戏是否暂停
    int speed = NORMAL_SPEED;  // 当前游戏速度

    while (true) {
        processInput(snake, paused, speed);  // 处理图形窗口中的输入

        if (!paused) {  // 如果游戏未暂停
            snake.move();  // 移动蛇身

            // 碰撞检测
            if (snake.checkCollision()) {
                settextcolor(RED);
                settextstyle(40, 0, _T("Arial"));
                outtextxy(WIDTH * BLOCK_SIZE / 2 - 100, HEIGHT * BLOCK_SIZE / 2 - 20, _T("GAME OVER!"));
                // 等待用户按下任意键退出
                ExMessage msg;
                while (true) {
                    if (peekmessage(&msg)) {
                        if (msg.message == WM_KEYDOWN) {
                            exit(0);
                        }
                    }
                }
            }

            // 吃食物检测
            if (snake.body.front().x == food.pos.x &&
                snake.body.front().y == food.pos.y) {
                snake.grow();
                foodCounter++;
                if (foodCounter % 2 == 0) {  // 每吃2个食物增加4个障碍物
                    obstacle.generate(snake, food, 4);
                }
                food.generate(snake, obstacle.obstacles);
            }
        }

        drawGame(snake, food, obstacle);    // 绘制游戏
        Sleep(speed);                       // 控制游戏速度
    }

    mciSendString(_T("close bgm"), NULL, 0, NULL);

    closegraph();  // 关闭图形窗口
    return 0;
}