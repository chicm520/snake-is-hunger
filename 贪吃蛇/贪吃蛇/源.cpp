#include <graphics.h>
#include <deque>
#include <time.h>
#include <mmsystem.h>
#include <vector>
#pragma comment(lib, "winmm.lib")

// ��Ϸ��������
const int BLOCK_SIZE = 20;    // ÿ�����������ش�С
const int WIDTH = 30;         // ��Ϸ�����ȣ���λ���飩
const int HEIGHT = 20;        // ��Ϸ����߶ȣ���λ���飩
const int NORMAL_SPEED = 150; // ������Ϸ�ٶȣ����룩
const int FAST_SPEED = 75;    // ���ٺ���Ϸ�ٶȣ����룩

// ����ö��
enum Direction { UP, DOWN, LEFT, RIGHT };

// ����
class Snake {
public:
    std::deque<POINT> body;   // ʹ��˫�˶��д洢��������
    Direction dir;            // ��ǰ�ƶ�����
    int score;                // ��Ϸ�÷�

    Snake() {
        // ��ʼ������3�ڣ�
        body.push_back({ WIDTH / 2, HEIGHT / 2 });
        body.push_back({ WIDTH / 2 - 1, HEIGHT / 2 });
        body.push_back({ WIDTH / 2 - 2, HEIGHT / 2 });
        dir = RIGHT;
        score = 0;
    }

    // �ƶ�����
    void move() {
        POINT head = body.front();
        POINT newHead = head;

        // ���ݷ��������ͷ��λ��
        switch (dir) {
        case UP:    newHead.y--; break;
        case DOWN:  newHead.y++; break;
        case LEFT:  newHead.x--; break;
        case RIGHT: newHead.x++; break;
        }

        body.push_front(newHead);  // �����ͷ��
        body.pop_back();           // �Ƴ�β�������ǳԵ�ʳ�
    }

    // ��������
    void grow() {
        body.push_back(body.back());
        score += 10;
    }

    // ��ײ���
    bool checkCollision() {
        POINT head = body.front();

        // �߽���
        if (head.x < 0 || head.x >= WIDTH || head.y < 0 || head.y >= HEIGHT)
            return true;

        // �������
        for (auto it = ++body.begin(); it != body.end(); ++it) {
            if (head.x == it->x && head.y == it->y)
                return true;
        }

        return false;
    }
};

// ʳ����
class Food {
public:
    POINT pos;

    Food(const Snake& snake, const std::vector<POINT>& obstacles) {
        generate(snake, obstacles);
    }

    // ������ʳ��ܿ�������ϰ��
    void generate(const Snake& snake, const std::vector<POINT>& obstacles) {
        bool onSnakeOrObstacle;
        do {
            onSnakeOrObstacle = false;
            pos.x = rand() % WIDTH;
            pos.y = rand() % HEIGHT;

            // ����Ƿ������������ϻ��ϰ�����
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

// �ϰ�����
class Obstacle {
public:
    std::vector<POINT> obstacles;

    // �������ϰ���ܿ������ʳ�
    void generate(const Snake& snake, const Food& food, int count) {
        for (int i = 0; i < count; i++) {
            bool onSnakeOrFood;
            POINT newObstacle;
            do {
                onSnakeOrFood = false;
                newObstacle.x = rand() % WIDTH;
                newObstacle.y = rand() % HEIGHT;

                // ����Ƿ������������ϻ�ʳ����
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

// ������Ϸ����
void drawGame(const Snake& snake, const Food& food, const Obstacle& obstacle) {
    cleardevice();  // �����Ļ

    // ����������
    setlinecolor(LIGHTGRAY);
    for (int i = 0; i <= WIDTH; i++)
        line(i * BLOCK_SIZE, 0, i * BLOCK_SIZE, HEIGHT * BLOCK_SIZE);
    for (int i = 0; i <= HEIGHT; i++)
        line(0, i * BLOCK_SIZE, WIDTH * BLOCK_SIZE, i * BLOCK_SIZE);

    // ����������ɫ���䣩
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

    // ����ʳ���ɫԲ�Σ�
    setfillcolor(RED);
    fillcircle(
        food.pos.x * BLOCK_SIZE + BLOCK_SIZE / 2,
        food.pos.y * BLOCK_SIZE + BLOCK_SIZE / 2,
        BLOCK_SIZE / 2 - 2
    );

    // �����ϰ����ɫ���飩
    setfillcolor(BLACK);
    for (const auto& obs : obstacle.obstacles) {
        fillrectangle(
            obs.x * BLOCK_SIZE + 1,
            obs.y * BLOCK_SIZE + 1,
            (obs.x + 1) * BLOCK_SIZE - 1,
            (obs.y + 1) * BLOCK_SIZE - 1
        );
    }

    // ��ʾ����
    settextcolor(BLACK);
    settextstyle(20, 0, _T("Arial"));
    TCHAR scoreText[20];
    sprintf_s(scoreText, _T("Score: %d"), snake.score);
    outtextxy(10, 10, scoreText);
}

// ����ͼ�δ����еļ��̺��������
void processInput(Snake& snake, bool& paused, int& speed) {
    ExMessage msg;
    while (peekmessage(&msg)) {
        if (msg.message == WM_KEYDOWN) {
            switch (msg.vkcode) {
            case 'w': case 'W':  // ��
                if (snake.dir != DOWN) snake.dir = UP;
                break;
            case 's': case 'S':  // ��
                if (snake.dir != UP) snake.dir = DOWN;
                break;
            case 'a': case 'A':  // ��
                if (snake.dir != RIGHT) snake.dir = LEFT;
                break;
            case 'd': case 'D':  // ��
                if (snake.dir != LEFT) snake.dir = RIGHT;
                break;
            case 'p': case 'P':  // P����ͣ/����
                paused = !paused;
                break;
            case 27:  // ESC���˳�
                exit(0);
            }
        }
        else if (msg.message == WM_LBUTTONDOWN) {  // ���������¼���
            speed = FAST_SPEED;
        }
        else if (msg.message == WM_LBUTTONUP) {    // �������ɿ��ָ������ٶ�
            speed = NORMAL_SPEED;
        }
    }
}

// ��ʾ��ʼ����
void showStartScreen() {
    cleardevice();  // �����Ļ
    setbkcolor(WHITE);  // ���ñ���ɫ
    settextcolor(BLACK);

    IMAGE background;
    loadimage(&background, _T("back.jpg"));  // ���ر���ͼƬ
    putimage(0, 0, &background);  // ���Ʊ���ͼƬ

    settextstyle(40, 0, _T("Arial"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 - 50, _T("̰������Ϸ"));
    settextstyle(20, 0, _T("Arial"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 150, HEIGHT * BLOCK_SIZE / 2 + 50, _T("���������ʼ��Ϸ"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 + 100, _T("�����'w''a''s''d'�����ߵ����������ƶ�"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 + 150, _T("ESC���˳�"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 + 200, _T("��ס����������"));
    outtextxy(WIDTH * BLOCK_SIZE / 2 - 200, HEIGHT * BLOCK_SIZE / 2 + 250, _T("P����ͣ/����"));

    // �ȴ��û������������ʼ��Ϸ
    while (true) {
        ExMessage msg;
        if (peekmessage(&msg)) {
            if (msg.message == WM_KEYDOWN) {
                return;  // ��ʼ��Ϸ
            }
        }
    }
}

int main() {
    initgraph(WIDTH * BLOCK_SIZE, HEIGHT * BLOCK_SIZE);  // ����ͼ�δ���
    setbkcolor(WHITE);                                   // ���ñ���ɫ
    srand((unsigned)time(NULL));                         // ��ʼ���������

    showStartScreen();                                   // ��ʾ��ʼ����

    // ���ű�������
    mciSendString(_T("open BGM.mp3 alias bgm"), NULL, 0, NULL);
    mciSendString(_T("play bgm repeat"), NULL, 0, NULL);

    Snake snake;
    Food food(snake, {});
    Obstacle obstacle;
    int foodCounter = 0;  // ��¼�Ե���ʳ������
    bool paused = false;   // ��Ϸ�Ƿ���ͣ
    int speed = NORMAL_SPEED;  // ��ǰ��Ϸ�ٶ�

    while (true) {
        processInput(snake, paused, speed);  // ����ͼ�δ����е�����

        if (!paused) {  // �����Ϸδ��ͣ
            snake.move();  // �ƶ�����

            // ��ײ���
            if (snake.checkCollision()) {
                settextcolor(RED);
                settextstyle(40, 0, _T("Arial"));
                outtextxy(WIDTH * BLOCK_SIZE / 2 - 100, HEIGHT * BLOCK_SIZE / 2 - 20, _T("GAME OVER!"));
                // �ȴ��û�����������˳�
                ExMessage msg;
                while (true) {
                    if (peekmessage(&msg)) {
                        if (msg.message == WM_KEYDOWN) {
                            exit(0);
                        }
                    }
                }
            }

            // ��ʳ����
            if (snake.body.front().x == food.pos.x &&
                snake.body.front().y == food.pos.y) {
                snake.grow();
                foodCounter++;
                if (foodCounter % 2 == 0) {  // ÿ��2��ʳ������4���ϰ���
                    obstacle.generate(snake, food, 4);
                }
                food.generate(snake, obstacle.obstacles);
            }
        }

        drawGame(snake, food, obstacle);    // ������Ϸ
        Sleep(speed);                       // ������Ϸ�ٶ�
    }

    mciSendString(_T("close bgm"), NULL, 0, NULL);

    closegraph();  // �ر�ͼ�δ���
    return 0;
}