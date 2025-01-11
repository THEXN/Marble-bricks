#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>  // �����Ч
#include <SDL_image.h>
#include <vector>
#include <iostream>
#include <string>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int BRICK_ROWS = 8;       // ����ש������
const int BRICK_COLS = 12;      // ����ש������
const int BRICK_WIDTH = 60;     // ��Сש����
const int BRICK_HEIGHT = 15;    // ��Сש��߶�

const int PADDLE_WIDTH = 80;    // ���̵�����
const int PADDLE_HEIGHT = 10;

const int BALL_SIZE = 10;       // ������Ĵ�С����


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

// ������Ϸ״̬
void resetGame(std::vector<Brick>& bricks, SDL_Rect& ball, SDL_Rect& paddle, int& ballVelX, int& ballVelY, int& score, int& lives) {
    bricks.clear();
    createBricks(bricks); // ��������ש��
    resetBall(ball, ballVelX, ballVelY); // �������λ�ú��ٶ�
    paddle.x = SCREEN_WIDTH / 2 - PADDLE_WIDTH / 2; // ���õ���λ��
    score = 0; // ���÷���
    lives = 3; // ��������ֵ
}

// ��Ⱦ�ı�
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y) {
    SDL_Color textColor = { 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    if (!textSurface) {
        std::cerr << "Unable to create text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "Unable to create text texture! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);  // ÿ����Ⱦ���ͷ�����
}

// �������ש���Ƿ�ݻ���
bool allBricksDestroyed(const std::vector<Brick>& bricks) {
    for (const auto& brick : bricks) {
        if (!brick.isDestroyed) {
            return false;  // ����δ�ݻٵ�ש��
        }
    }
    return true;  // ����ש�鶼�ݻ���
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();  // ��ʼ�� SDL_ttf ��
    Mix_Init(MIX_INIT_MP3); // ��ʼ����Ƶ
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);  // ��ʼ����Ƶ�豸

    // ��������
    SDL_Window* window = SDL_CreateWindow("Breakout Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // ����ͼ��
    SDL_Surface* icon = IMG_Load("./Content/icon.png");  // ʹ�� SDL_image ����ͼ��
    if (!icon) {
        std::cerr << "Failed to load icon: " << IMG_GetError() << std::endl;
        return -1;
    }

    // ���ô���ͼ��
    SDL_SetWindowIcon(window, icon);

    // �ͷ�ͼ����Դ
    SDL_FreeSurface(icon);


    // ��������
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/Consola.ttf", 24);  // �滻Ϊ�������·��
    if (font == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    // ������Ч
    Mix_Chunk* brickHitSound = Mix_LoadWAV("./Content/hit.wav");
    Mix_Chunk* ballMissedSound = Mix_LoadWAV("./Content/life.wav");
    Mix_Chunk* winSound = Mix_LoadWAV("./Content/win.wav");
    Mix_Chunk* gameOverSound = Mix_LoadWAV("./Content/gameOver.wav");

    if (!brickHitSound || !ballMissedSound || !winSound || !gameOverSound) {
        std::cerr << "Failed to load sound effects! SDL_mixer Error: " << Mix_GetError() << std::endl;
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
        }

        // ����ש�����ײ
        for (auto& brick : bricks) {
            if (!brick.isDestroyed && SDL_HasIntersection(&ball, &brick.rect)) {
                brick.isDestroyed = true;
                ballVelY = -ballVelY;
                score += 10; // ÿ��ש��÷�
                Mix_PlayChannel(-1, brickHitSound, 0); // ����ש����ײ��Ч
                break;
            }
        }

        // ����Ƿ�����ש�鶼���ݻ�
        if (allBricksDestroyed(bricks)) {
            Mix_PlayChannel(-1, winSound, 0); // ����ʤ����Ч
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // ��ɫ����
            SDL_RenderClear(renderer);
            renderText(renderer, font, "You Win! Press Enter for Next Level", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
            SDL_RenderPresent(renderer);

            bool waiting = true;
            while (waiting) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        running = false;
                        waiting = false;
                    }
                    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                        // ���¿�ʼ�ؿ�
                        resetGame(bricks, ball, paddle, ballVelX, ballVelY, score, lives);
                        waiting = false;
                    }
                }
            }
        }

        // �������Ļ
        if (ball.y > SCREEN_HEIGHT) {
            lives--; // ��������ֵ
            Mix_PlayChannel(-1, ballMissedSound, 0); // �����������Ч
            if (lives <= 0) {
                // ��Ϸ�����߼�
                Mix_PlayChannel(-1, gameOverSound, 0); // ������Ϸʧ����Ч
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // ��ɫ����
                SDL_RenderClear(renderer);
                renderText(renderer, font, "Game Over! Your score: " + std::to_string(score), SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 - 30);
                renderText(renderer, font, "Press ENTER to restart", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 + 10);
                SDL_RenderPresent(renderer);

                // �ȴ���Ұ��»س���
                bool waiting = true;
                while (waiting) {
                    while (SDL_PollEvent(&event)) {
                        if (event.type == SDL_QUIT) {
                            running = false;
                            waiting = false;
                        }
                        if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
                            resetGame(bricks, ball, paddle, ballVelX, ballVelY, score, lives);
                            waiting = false;
                        }
                    }
                    SDL_Delay(10);
                }
            }
            else {
                resetBall(ball, ballVelX, ballVelY); // �������λ�ú��ٶ�
            }
        }

        // ��ÿ֡��ʼ֮ǰ�����Ļ
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // ���ñ�����ɫΪ��ɫ
        SDL_RenderClear(renderer);

        // ���Ƶ���
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // ��ɫ����
        SDL_RenderFillRect(renderer, &paddle);

        // ������
        SDL_RenderFillRect(renderer, &ball);

        // ����ש��
        for (const auto& brick : bricks) {
            if (!brick.isDestroyed) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // ��ɫש��
                SDL_RenderFillRect(renderer, &brick.rect);
            }
        }

        // ��Ⱦ����������ֵ
        std::string scoreText = "Score: " + std::to_string(score) + "  Lives: " + std::to_string(lives);
        renderText(renderer, font, scoreText, 10, 10);

        // ˢ����Ⱦ
        SDL_RenderPresent(renderer);

        // ����ӳ��Կ���֡��
        SDL_Delay(10); // Լ60 FPS
    }

    // ����
    Mix_FreeChunk(brickHitSound);
    Mix_FreeChunk(ballMissedSound);
    Mix_FreeChunk(winSound);
    Mix_FreeChunk(gameOverSound);
    Mix_Quit();  // �˳� SDL_mixer
    TTF_CloseFont(font);  // �ر�����
    TTF_Quit();  // �˳� SDL_ttf
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
