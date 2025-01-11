#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>  // 添加音效
#include <SDL_image.h>
#include <vector>
#include <iostream>
#include <string>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int BRICK_ROWS = 8;       // 增加砖块行数
const int BRICK_COLS = 12;      // 增加砖块列数
const int BRICK_WIDTH = 60;     // 缩小砖块宽度
const int BRICK_HEIGHT = 15;    // 缩小砖块高度

const int PADDLE_WIDTH = 80;    // 缩短挡板宽度
const int PADDLE_HEIGHT = 10;

const int BALL_SIZE = 10;       // 保持球的大小不变


// 砖块结构体
struct Brick {
    SDL_Rect rect;  // 砖块的矩形区域
    bool isDestroyed;  // 砖块是否被摧毁
};

// 创建砖块
void createBricks(std::vector<Brick>& bricks) {
    for (int row = 0; row < BRICK_ROWS; ++row) {
        for (int col = 0; col < BRICK_COLS; ++col) {
            Brick brick = { {col * (BRICK_WIDTH + 10) + 35, row * (BRICK_HEIGHT + 10) + 50, BRICK_WIDTH, BRICK_HEIGHT}, false };
            bricks.push_back(brick);
        }
    }
}

// 重置球的位置和速度
void resetBall(SDL_Rect& ball, int& ballVelX, int& ballVelY) {
    ball.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
    ball.y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
    ballVelX = 5;
    ballVelY = -5;
}

// 重置游戏状态
void resetGame(std::vector<Brick>& bricks, SDL_Rect& ball, SDL_Rect& paddle, int& ballVelX, int& ballVelY, int& score, int& lives) {
    bricks.clear();
    createBricks(bricks); // 重新生成砖块
    resetBall(ball, ballVelX, ballVelY); // 重置球的位置和速度
    paddle.x = SCREEN_WIDTH / 2 - PADDLE_WIDTH / 2; // 重置挡板位置
    score = 0; // 重置分数
    lives = 3; // 重置生命值
}

// 渲染文本
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
    SDL_DestroyTexture(textTexture);  // 每次渲染后释放纹理
}

// 检查所有砖块是否摧毁完
bool allBricksDestroyed(const std::vector<Brick>& bricks) {
    for (const auto& brick : bricks) {
        if (!brick.isDestroyed) {
            return false;  // 仍有未摧毁的砖块
        }
    }
    return true;  // 所有砖块都摧毁了
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();  // 初始化 SDL_ttf 库
    Mix_Init(MIX_INIT_MP3); // 初始化音频
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);  // 初始化音频设备

    // 创建窗口
    SDL_Window* window = SDL_CreateWindow("Breakout Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 加载图标
    SDL_Surface* icon = IMG_Load("./Content/icon.png");  // 使用 SDL_image 加载图标
    if (!icon) {
        std::cerr << "Failed to load icon: " << IMG_GetError() << std::endl;
        return -1;
    }

    // 设置窗口图标
    SDL_SetWindowIcon(window, icon);

    // 释放图标资源
    SDL_FreeSurface(icon);


    // 加载字体
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/Consola.ttf", 24);  // 替换为你的字体路径
    if (font == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    // 加载音效
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
    int lives = 3; // 玩家初始生命值

    bool running = true;
    SDL_Event event;

    while (running) {
        // 事件处理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // 挡板移动
        const Uint8* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_LEFT] && paddle.x > 0) {
            paddle.x -= 10;
        }
        if (state[SDL_SCANCODE_RIGHT] && paddle.x < SCREEN_WIDTH - PADDLE_WIDTH) {
            paddle.x += 10;
        }

        // 球移动
        ball.x += ballVelX;
        ball.y += ballVelY;

        // 球与墙的碰撞
        if (ball.x <= 0 || ball.x + BALL_SIZE >= SCREEN_WIDTH) {
            ballVelX = -ballVelX;
        }
        if (ball.y <= 0) {
            ballVelY = -ballVelY;
        }

        // 球与挡板的碰撞
        if (SDL_HasIntersection(&ball, &paddle)) {
            ballVelY = -ballVelY;
        }

        // 球与砖块的碰撞
        for (auto& brick : bricks) {
            if (!brick.isDestroyed && SDL_HasIntersection(&ball, &brick.rect)) {
                brick.isDestroyed = true;
                ballVelY = -ballVelY;
                score += 10; // 每个砖块得分
                Mix_PlayChannel(-1, brickHitSound, 0); // 播放砖块碰撞音效
                break;
            }
        }

        // 检查是否所有砖块都被摧毁
        if (allBricksDestroyed(bricks)) {
            Mix_PlayChannel(-1, winSound, 0); // 播放胜利音效
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 黑色背景
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
                        // 重新开始关卡
                        resetGame(bricks, ball, paddle, ballVelX, ballVelY, score, lives);
                        waiting = false;
                    }
                }
            }
        }

        // 球落出屏幕
        if (ball.y > SCREEN_HEIGHT) {
            lives--; // 减少生命值
            Mix_PlayChannel(-1, ballMissedSound, 0); // 播放球掉落音效
            if (lives <= 0) {
                // 游戏结束逻辑
                Mix_PlayChannel(-1, gameOverSound, 0); // 播放游戏失败音效
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 黑色背景
                SDL_RenderClear(renderer);
                renderText(renderer, font, "Game Over! Your score: " + std::to_string(score), SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 - 30);
                renderText(renderer, font, "Press ENTER to restart", SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 + 10);
                SDL_RenderPresent(renderer);

                // 等待玩家按下回车键
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
                resetBall(ball, ballVelX, ballVelY); // 重置球的位置和速度
            }
        }

        // 在每帧开始之前清空屏幕
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // 设置背景颜色为黑色
        SDL_RenderClear(renderer);

        // 绘制挡板
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // 白色挡板
        SDL_RenderFillRect(renderer, &paddle);

        // 绘制球
        SDL_RenderFillRect(renderer, &ball);

        // 绘制砖块
        for (const auto& brick : bricks) {
            if (!brick.isDestroyed) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // 红色砖块
                SDL_RenderFillRect(renderer, &brick.rect);
            }
        }

        // 渲染分数和生命值
        std::string scoreText = "Score: " + std::to_string(score) + "  Lives: " + std::to_string(lives);
        renderText(renderer, font, scoreText, 10, 10);

        // 刷新渲染
        SDL_RenderPresent(renderer);

        // 添加延迟以控制帧率
        SDL_Delay(10); // 约60 FPS
    }

    // 清理
    Mix_FreeChunk(brickHitSound);
    Mix_FreeChunk(ballMissedSound);
    Mix_FreeChunk(winSound);
    Mix_FreeChunk(gameOverSound);
    Mix_Quit();  // 退出 SDL_mixer
    TTF_CloseFont(font);  // 关闭字体
    TTF_Quit();  // 退出 SDL_ttf
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
