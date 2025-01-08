#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <iostream>
#include <string>

const int SCREEN_WIDTH = 800;  // ��Ļ����
const int SCREEN_HEIGHT = 600;  // ��Ļ�߶�

const int BRICK_ROWS = 5;  // ש������
const int BRICK_COLS = 10;  // ש������
const int BRICK_WIDTH = 70;  // ש�����
const int BRICK_HEIGHT = 20;  // ש��߶�

const int PADDLE_WIDTH = 100;  // �������
const int PADDLE_HEIGHT = 10;  // ����߶�

const int BALL_SIZE = 10;  // ��Ĵ�С

// ש��ṹ��
struct Brick {
    SDL_Rect rect;  // ש��ľ�������
    bool isDestroyed;  // ש���Ƿ񱻴ݻ�
};

// ����ש��
void createBricks(std::vector<Brick>& bricks) {
    for (int row = 0; row < BRICK_ROWS; ++row) {
        for (int col = 0; col < BRICK_COLS; ++col) {
            Brick brick = { {col * (BRICK_WIDTH + 10) + 35, row * (BRICK_HEIGHT + 10) + 50, BRICK_WIDTH, BRICK_HEIGHT}, false };
            bricks.push_back(brick);
        }
    }
}

// �������λ�ú��ٶ�
void resetBall(SDL_Rect& ball, int& ballVelX, int& ballVelY) {
    ball.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
    ball.y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
    ballVelX = 5;
    ballVelY = -5;
}

// ��Ⱦ�ı�
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y) {
    SDL_Color textColor = { 255, 255, 255 }; // ��ɫ
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    if (textSurface == nullptr) {
        std::cerr << "Unable to create text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);  // ��Ⱦ���ͷű���

    SDL_Rect textRect = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);  // ��Ⱦ�ı�����
    SDL_DestroyTexture(textTexture);  // ��Ⱦ����������
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();  // ��ʼ�� SDL_ttf ��

    SDL_Window* window = SDL_CreateWindow("Breakout Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // ��������
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/Consola.ttf", 24);  // �滻Ϊ�������·��
    if (font == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    SDL_Rect paddle = { SCREEN_WIDTH / 2 - PADDLE_WIDTH / 2, SCREEN_HEIGHT - 50, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_Rect ball = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, BALL_SIZE, BALL_SIZE };
    int ballVelX = 5, ballVelY = -5;

    std::vector<Brick> bricks;
    createBricks(bricks);

    int score = 0;
    int lives = 3; // ��ҳ�ʼ����ֵ

    bool running = true;
    SDL_Event event;

    while (running) {
        // �¼�����
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // �����ƶ�
        const Uint8* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_LEFT] && paddle.x > 0) {
            paddle.x -= 10;
        }
        if (state[SDL_SCANCODE_RIGHT] && paddle.x < SCREEN_WIDTH - PADDLE_WIDTH) {
            paddle.x += 10;
        }

        // ���ƶ�
        ball.x += ballVelX;
        ball.y += ballVelY;

        // ����ǽ����ײ
        if (ball.x <= 0 || ball.x + BALL_SIZE >= SCREEN_WIDTH) {
            ballVelX = -ballVelX;
        }
        if (ball.y <= 0) {
            ballVelY = -ballVelY;
        }

        // ���뵲�����ײ
        if (SDL_HasIntersection(&ball, &paddle)) {
            ballVelY = -ballVelY;
            ball.y = paddle.y - BALL_SIZE; // ��ֹճ��
        }

        // ����ש�����ײ
        for (auto& brick : bricks) {
            if (!brick.isDestroyed && SDL_HasIntersection(&ball, &brick.rect)) {
                brick.isDestroyed = true;
                ballVelY = -ballVelY;
                score += 10; // ÿ��ש��÷�
                break;
            }
        }

        // �������Ļ
        if (ball.y > SCREEN_HEIGHT) {
            lives--; // ��������ֵ
            if (lives <= 0) {
                running = false; // ����ֵ�ľ�����Ϸ����
            }
            else {
                resetBall(ball, ballVelX, ballVelY); // �������λ�ú��ٶ�
            }
        }

        // ��ÿһ֡��ʼ֮ǰ�������
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // ���Ƶ���
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &paddle);

        // ������
        SDL_RenderFillRect(renderer, &ball);

        // ����ש��
        for (const auto& brick : bricks) {
            if (!brick.isDestroyed) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderFillRect(renderer, &brick.rect);
            }
        }

        // ��Ⱦ����������ֵ
        renderText(renderer, font, "Score: " + std::to_string(score) + "  Lives: " + std::to_string(lives), 10, 10);

        // ˢ����Ⱦ
        SDL_RenderPresent(renderer);

        // �����ӳ��Կ���֡��
        SDL_Delay(10); // Լ60 FPS
    }

    // ��Ϸ��������ʾ���յ÷�
    renderText(renderer, font, "Game Over! Your score: " + std::to_string(score), SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000); // ��ʾ 3 ����

    // ����
    TTF_CloseFont(font);  // �ر�����
    TTF_Quit();  // �˳� SDL_ttf
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}