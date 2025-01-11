#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <iostream>
#include <string>

const int SCREEN_WIDTH = 800;  // 屏幕宽度
const int SCREEN_HEIGHT = 600;  // 屏幕高度

const int BRICK_ROWS = 5;  // 砖块行数
const int BRICK_COLS = 10;  // 砖块列数
const int BRICK_WIDTH = 70;  // 砖块宽度
const int BRICK_HEIGHT = 20;  // 砖块高度

const int PADDLE_WIDTH = 100;  // 挡板宽度
const int PADDLE_HEIGHT = 10;  // 挡板高度

const int BALL_SIZE = 10;  // 球的大小

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


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();  // 初始化 SDL_ttf 库

    SDL_Window* window = SDL_CreateWindow("Breakout Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 加载字体
    TTF_Font* font = TTF_OpenFont("C:/Windows/Fonts/Consola.ttf", 24);  // 替换为你的字体路径
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
            ball.y = paddle.y - BALL_SIZE; // 防止粘连
        }

        // 球与砖块的碰撞
        for (auto& brick : bricks) {
            if (!brick.isDestroyed && SDL_HasIntersection(&ball, &brick.rect)) {
                brick.isDestroyed = true;
                ballVelY = -ballVelY;
                score += 10; // 每个砖块得分
                break;
            }
        }

        // 球落出屏幕
        if (ball.y > SCREEN_HEIGHT) {
            lives--; // 减少生命值
            if (lives <= 0) {
                running = false; // 生命值耗尽，游戏结束
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

    // 游戏结束，显示最终得分
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 黑色背景
    SDL_RenderClear(renderer);
    renderText(renderer, font, "Game Over! Your score: " + std::to_string(score), SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000); // 显示3秒


    // 清理
    TTF_CloseFont(font);  // 关闭字体
    TTF_Quit();  // 退出 SDL_ttf
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
