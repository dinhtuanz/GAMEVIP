#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <ctime>

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 700;

struct Button {
    SDL_Rect rect;
    SDL_Color color;
    const char* text;
};

struct Obstacle {
    SDL_Rect rect;
    int speed;
    bool passed;
    int textureIndex; // Thêm trường để lưu chỉ số texture
};

class Game {
private:
    SDL_Rect ballRect;
    SDL_Texture* ballTexture;
    std::vector<Obstacle> obstacles;
    std::vector<SDL_Texture*> obstacleTextures; // Thêm vector lưu texture chướng ngại vật
    
    int score;
    bool isGameOver;
    Mix_Chunk* crashSound;
    Mix_Chunk* scoreSound;
    int baseSpeed;
    std::mt19937 rng;
    
    // Giới hạn game
    const int MAX_OBSTACLES = 8;
    const int MAX_SPEED = 20;
    const int OBSTACLE_SIZE = 30;
    
public:
    Game() : score(0), isGameOver(false), baseSpeed(2), rng(std::time(nullptr)) {}
    
    void loadObstacleTextures(SDL_Renderer* renderer) {
        for (int i = 1; i <= 7; i++) {
            std::string path = "assets/images/obstacles/" + std::to_string(i) + ".png";
            SDL_Surface* surface = IMG_Load(path.c_str());
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                obstacleTextures.push_back(texture);
            } else {
                std::cerr << "Failed to load obstacle image: " << path << " - " << IMG_GetError() << std::endl;
                obstacleTextures.push_back(nullptr);
            }
        }
    }
    
    void init(SDL_Renderer* renderer, const std::string& ballPath, 
              const std::string& crashSoundPath, const std::string& scoreSoundPath) {
        // Load ball texture
        isGameOver = false;
        SDL_Surface* surface = IMG_Load(ballPath.c_str());
        if (surface) {
            ballTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
        
        // Set initial ball position
        ballRect = {SCREEN_WIDTH/2 - 25, SCREEN_HEIGHT - 100, 50, 50};
        
        // Load sounds
        crashSound = Mix_LoadWAV(crashSoundPath.c_str());
        scoreSound = Mix_LoadWAV(scoreSoundPath.c_str());
        
        // Load obstacle textures
        loadObstacleTextures(renderer);
        
        // Generate initial obstacles
        generateObstacles(3);
    }
    
    void generateObstacles(int count) {
        if (obstacles.size() >= MAX_OBSTACLES) return;

        std::uniform_int_distribution<int> distX(0, SCREEN_WIDTH - OBSTACLE_SIZE);
        std::uniform_int_distribution<int> distSpeed(1, baseSpeed);
        std::uniform_int_distribution<int> distTexture(0, obstacleTextures.size() - 1);
        
        int obstaclesToCreate = std::min(count, MAX_OBSTACLES - (int)obstacles.size());
        
        for (int i = 0; i < obstaclesToCreate; i++) {
            obstacles.push_back({
                {distX(rng), -OBSTACLE_SIZE - (i * 150), OBSTACLE_SIZE, OBSTACLE_SIZE},
                distSpeed(rng),
                false,
                distTexture(rng) // Gán texture ngẫu nhiên
            });
        }
    }
    
    void handleEvent(SDL_Event* e) {
        if (isGameOver) {
            if (e->type == SDL_MOUSEBUTTONDOWN) {
                reset();
            }
            return;
        }
        
        if (e->type == SDL_MOUSEMOTION) {
            ballRect.x = e->motion.x - ballRect.w/2;
            
            // Giới hạn không cho ra khỏi màn hình
            if (ballRect.x < 0) ballRect.x = 0;
            if (ballRect.x > SCREEN_WIDTH - ballRect.w) ballRect.x = SCREEN_WIDTH - ballRect.w;
        }
    }
    
    void update() {
        if (isGameOver) return;
        
        for (auto& obstacle : obstacles) {
            obstacle.rect.y += obstacle.speed;
            
            if (SDL_HasIntersection(&ballRect, &obstacle.rect)) {
                isGameOver = true;
                if (crashSound) Mix_PlayChannel(-1, crashSound, 0);
                return;
            }
            
            if (!obstacle.passed && obstacle.rect.y > SCREEN_HEIGHT) {
                obstacle.passed = true;
                score++;
                if (scoreSound) Mix_PlayChannel(-1, scoreSound, 0);
            }
        }
        
        // Tạo chướng ngại vật mới khi cần
        if (obstacles.size() < MAX_OBSTACLES / 2 || 
            (obstacles.back().rect.y > -OBSTACLE_SIZE && obstacles.size() < MAX_OBSTACLES)) {
            generateObstacles(1);
        }
        
        // Xóa chướng ngại vật đã qua
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(),
                [](const Obstacle& o) { return o.rect.y > SCREEN_HEIGHT && o.passed; }),
            obstacles.end()
        );
        
        // Tăng độ khó mỗi 10 điểm
        if (score % 10 == 0 && score > 0) {
            baseSpeed = std::min(2 + score/10, MAX_SPEED);
        }
    }
    
    void render(SDL_Renderer* renderer, TTF_Font* font) {
        // Vẽ quả bóng
        if (ballTexture) {
            SDL_RenderCopy(renderer, ballTexture, NULL, &ballRect);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &ballRect);
        }
        
        // Vẽ chướng ngại vật với texture
        for (const auto& obstacle : obstacles) {
            if (!obstacleTextures.empty() && obstacle.textureIndex < obstacleTextures.size() && 
                obstacleTextures[obstacle.textureIndex]) {
                SDL_RenderCopy(renderer, obstacleTextures[obstacle.textureIndex], NULL, &obstacle.rect);
            } else {
                // Fallback nếu không có texture
                SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
                SDL_RenderFillRect(renderer, &obstacle.rect);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &obstacle.rect);
            }
        }
        
        // Vẽ điểm
        if (font) {
            std::string scoreText = "Score: " + std::to_string(score);
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }
        
        // Màn hình kết thúc
        if (isGameOver) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &overlay);
            
            if (font) {
                SDL_Color textColor = {255, 255, 255, 255};
                
                // Game Over
                std::string gameOverText = "Game Over!";
                SDL_Surface* gameOverSurface = TTF_RenderText_Solid(font, gameOverText.c_str(), textColor);
                if (gameOverSurface) {
                    SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
                    if (gameOverTexture) {
                        SDL_Rect gameOverRect = {
                            (SCREEN_WIDTH - gameOverSurface->w)/2,
                            SCREEN_HEIGHT/2 - 50,
                            gameOverSurface->w,
                            gameOverSurface->h
                        };
                        SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
                        SDL_DestroyTexture(gameOverTexture);
                    }
                    SDL_FreeSurface(gameOverSurface);
                }
                
                // Điểm
                std::string scoreText = "Score: " + std::to_string(score);
                SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
                if (scoreSurface) {
                    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
                    if (scoreTexture) {
                        SDL_Rect scoreRect = {
                            (SCREEN_WIDTH - scoreSurface->w)/2,
                            SCREEN_HEIGHT/2,
                            scoreSurface->w,
                            scoreSurface->h
                        };
                        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
                        SDL_DestroyTexture(scoreTexture);
                    }
                    SDL_FreeSurface(scoreSurface);
                }
                
                // Hướng dẫn
                std::string instructionText = "Click to play again";
                SDL_Surface* instructionSurface = TTF_RenderText_Solid(font, instructionText.c_str(), textColor);
                if (instructionSurface) {
                    SDL_Texture* instructionTexture = SDL_CreateTextureFromSurface(renderer, instructionSurface);
                    if (instructionTexture) {
                        SDL_Rect instructionRect = {
                            (SCREEN_WIDTH - instructionSurface->w)/2,
                            SCREEN_HEIGHT/2 + 50,
                            instructionSurface->w,
                            instructionSurface->h
                        };
                        SDL_RenderCopy(renderer, instructionTexture, NULL, &instructionRect);
                        SDL_DestroyTexture(instructionTexture);
                    }
                    SDL_FreeSurface(instructionSurface);
                }
            }
        }
    }
    
    void reset() {
        obstacles.clear();
        score = 0;
        isGameOver = false;
        baseSpeed = 2;
        generateObstacles(3);
    }
    
    bool gameOver() const {
        return isGameOver;
    }
    
    ~Game() {
        if (ballTexture) SDL_DestroyTexture(ballTexture);
        if (crashSound) Mix_FreeChunk(crashSound);
        if (scoreSound) Mix_FreeChunk(scoreSound);
        for (auto texture : obstacleTextures) {
            if (texture) SDL_DestroyTexture(texture);
        }
    }
};

class BallSelector {
private:
    std::vector<SDL_Texture*> ballTextures;
    std::vector<std::string> ballPaths;
    int selectedIndex = 0;
    SDL_Rect ballRect = { (SCREEN_WIDTH - 100)/2, 250, 100, 100 };
    SDL_Rect leftArrowRect = { ballRect.x - 50, ballRect.y + (ballRect.h - 30)/2, 30, 30 };
    SDL_Rect rightArrowRect = { ballRect.x + ballRect.w + 20, ballRect.y + (ballRect.h - 30)/2, 30, 30 };
    Mix_Chunk* selectSound = nullptr;
    
public:
    bool loadResources(SDL_Renderer* renderer, const std::vector<std::string>& paths, const std::string& soundPath) {
        selectSound = Mix_LoadWAV(soundPath.c_str());
        if (!selectSound) {
            std::cerr << "Failed to load select sound: " << Mix_GetError() << std::endl;
        }

        for (const auto& path : paths) {
            SDL_Surface* surface = IMG_Load(path.c_str());
            if (!surface) {
                std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
                continue;
            }
            
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            
            if (texture) {
                ballTextures.push_back(texture);
                ballPaths.push_back(path);
            }
        }
        return !ballTextures.empty();
    }
    
    void handleEvent(SDL_Event* e) {
        if (e->type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            
            if (x >= leftArrowRect.x && x <= leftArrowRect.x + leftArrowRect.w &&
                y >= leftArrowRect.y && y <= leftArrowRect.y + leftArrowRect.h) {
                selectedIndex = (selectedIndex - 1 + ballTextures.size()) % ballTextures.size();
                if (selectSound) Mix_PlayChannel(-1, selectSound, 0);
            }
            else if (x >= rightArrowRect.x && x <= rightArrowRect.x + rightArrowRect.w &&
                     y >= rightArrowRect.y && y <= rightArrowRect.y + rightArrowRect.h) {
                selectedIndex = (selectedIndex + 1) % ballTextures.size();
                if (selectSound) Mix_PlayChannel(-1, selectSound, 0);
            }
        }
    }
    
    void render(SDL_Renderer* renderer, TTF_Font* font) {
        if (!ballTextures.empty()) {
            // Draw selected ball
            SDL_RenderCopy(renderer, ballTextures[selectedIndex], NULL, &ballRect);
            
            // Draw arrows
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderFillRect(renderer, &leftArrowRect);
            SDL_RenderFillRect(renderer, &rightArrowRect);
            
            // Draw arrow shapes
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            // Left arrow
            SDL_RenderDrawLine(renderer, leftArrowRect.x + 10, leftArrowRect.y + 15, 
                             leftArrowRect.x + 20, leftArrowRect.y + 5);
            SDL_RenderDrawLine(renderer, leftArrowRect.x + 10, leftArrowRect.y + 15, 
                             leftArrowRect.x + 20, leftArrowRect.y + 25);
            // Right arrow
            SDL_RenderDrawLine(renderer, rightArrowRect.x + 10, rightArrowRect.y + 5, 
                             rightArrowRect.x + 20, rightArrowRect.y + 15);
            SDL_RenderDrawLine(renderer, rightArrowRect.x + 10, rightArrowRect.y + 25, 
                             rightArrowRect.x + 20, rightArrowRect.y + 15);

            // Draw "Select Character" text below the ball
            if (font) {
                SDL_Color textColor = {255, 255, 255, 255};
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Select Character", textColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture) {
                        int textX = (SCREEN_WIDTH - textSurface->w) / 2;
                        int textY = ballRect.y + ballRect.h + 20;
                        SDL_Rect textRect = {textX, textY, textSurface->w, textSurface->h};
                        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_FreeSurface(textSurface);
                }
            }
        }
    }
    
    std::string getSelectedBallPath() const {
        return ballPaths.empty() ? "" : ballPaths[selectedIndex];
    }
    
    ~BallSelector() {
        for (auto texture : ballTextures) {
            SDL_DestroyTexture(texture);
        }
        if (selectSound) Mix_FreeChunk(selectSound);
    }
};

SDL_Texture* LoadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        std::cerr << "Failed to create texture: " << path << " - " << SDL_GetError() << std::endl;
    }
    
    return texture;
}

void DrawButton(SDL_Renderer* renderer, const Button& button, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, button.color.r, button.color.g, button.color.b, button.color.a);
    SDL_RenderFillRect(renderer, &button.rect);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &button.rect);

    if (font && button.text) {
        SDL_Color textColor = { 255, 255, 255, 255 };
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, button.text, textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                int textX = button.rect.x + (button.rect.w - textSurface->w) / 2;
                int textY = button.rect.y + (button.rect.h - textSurface->h) / 2;
                SDL_Rect textRect = { textX, textY, textSurface->w, textSurface->h };
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf initialization failed: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
    }

    // Load sounds
    Mix_Music* bgMusic = Mix_LoadMUS("assets/sounds/background.mp3");
    Mix_Chunk* buttonSound = Mix_LoadWAV("assets/sounds/button.mp3");
    Mix_Chunk* crashSound = Mix_LoadWAV("assets/sounds/crash.mp3");
    Mix_Chunk* scoreSound = Mix_LoadWAV("assets/sounds/score.mp3");
    
    if (bgMusic) {
        Mix_PlayMusic(bgMusic, -1); // Loop indefinitely
        Mix_VolumeMusic(MIX_MAX_VOLUME / 2); // Reduce volume
    }

    SDL_Window* window = SDL_CreateWindow(
        "Rapid Roll - Ball Selection",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    // Load fonts
    TTF_Font* font = TTF_OpenFont("assets/fonts/1.ttf", 24);
    TTF_Font* titleFont = TTF_OpenFont("assets/fonts/1.ttf", 100);
    TTF_Font* selectFont = TTF_OpenFont("assets/fonts/1.ttf", 28);
    if (!font || !titleFont || !selectFont) {
        std::cerr << "Failed to load fonts: " << TTF_GetError() << std::endl;
    }

    // Load background
    SDL_Texture* background = LoadTexture("assets/images/background.jpg", renderer);
    SDL_Texture* gameBackground = LoadTexture("assets/images/game_background.jpg", renderer);

    // Initialize ball selector
    BallSelector ballSelector;
    if (!ballSelector.loadResources(renderer, 
        {"assets/images/balls/traidat.jpg", "assets/images/ballschot8.jpg", 
         "assets/images/balls/nhu.jpg", "assets/images/balls/doivl.jpg", 
         "assets/images/balls/nguoiyeuem.jpg"},
        "assets/sounds/select.mp3")) {
        std::cerr << "Failed to load ball resources!" << std::endl;
    }

    // Create buttons
    Button buttons[3] = {
        {{150, 450, 200, 50}, {0, 255, 0, 255}, "Play"},
        {{150, 520, 200, 50}, {255, 165, 0, 255}, "Guide"},
        {{150, 590, 200, 50}, {255, 0, 0, 255}, "Settings"}
    };

    // Game state
    enum class GameState { MENU, PLAYING, GUIDE, SETTINGS };
    GameState currentState = GameState::MENU;
    Game game;

    bool isRunning = true;
    SDL_Event event;
    
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            
            switch (currentState) {
                case GameState::MENU:
                    if (event.type == SDL_MOUSEBUTTONDOWN) {
                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        
                        // Handle ball selection
                        ballSelector.handleEvent(&event);
                        
                        // Handle buttons
                        for (int i = 0; i < 3; i++) {
                            if (mouseX >= buttons[i].rect.x && mouseX <= buttons[i].rect.x + buttons[i].rect.w &&
                                mouseY >= buttons[i].rect.y && mouseY <= buttons[i].rect.y + buttons[i].rect.h) {
                                if (buttonSound) Mix_PlayChannel(-1, buttonSound, 0);
                                
                                if (i == 0) { // Play button
                                    game.init(renderer, ballSelector.getSelectedBallPath(),
                                              "assets/sounds/select.mp3", "assets/sounds/select.mp3");
                                    game.reset();
                                    currentState = GameState::PLAYING;
                                } else if (i == 1) { // Guide button
                                    currentState = GameState::GUIDE;
                                } else if (i == 2) { // Settings button
                                    currentState = GameState::SETTINGS;
                                }
                            }
                        }
                    }
                    break;
                    
                case GameState::PLAYING:
                    if (game.gameOver()) {
                        if (event.type == SDL_MOUSEBUTTONDOWN) {
                            currentState = GameState::MENU;
                        }
                    } else {
                        game.handleEvent(&event);
                    }
                    break;
                    
                case GameState::GUIDE:
                case GameState::SETTINGS:
                    if (event.type == SDL_MOUSEBUTTONDOWN) {
                        currentState = GameState::MENU;
                    }
                    break;
            }
        }

        // Update game state
        if (currentState == GameState::PLAYING && !game.gameOver()) {
            game.update();
        }

        // Rendering
        SDL_RenderClear(renderer);
        
        switch (currentState) {
            case GameState::MENU:
                // Draw menu background
                if (background) {
                    SDL_RenderCopy(renderer, background, NULL, NULL);
                }

                // Draw title
                if (titleFont) {
                    SDL_Color titleColor = {255, 255, 255, 255};
                    SDL_Surface* titleSurface = TTF_RenderText_Solid(titleFont, "RAPID ROLL", titleColor);
                    if (titleSurface) {
                        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
                        if (titleTexture) {
                            int titleX = (SCREEN_WIDTH - titleSurface->w) / 2;
                            SDL_Rect titleRect = {titleX, 100, titleSurface->w, titleSurface->h};
                            SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
                            SDL_DestroyTexture(titleTexture);
                        }
                        SDL_FreeSurface(titleSurface);
                    }
                }
                
                // Draw ball selector
                ballSelector.render(renderer, selectFont);
                
                // Draw buttons
                for (const auto& button : buttons) {
                    DrawButton(renderer, button, font);
                }
                break;
                
            case GameState::PLAYING:
                // Draw game background
                if (gameBackground) {
                    SDL_RenderCopy(renderer, gameBackground, NULL, NULL);
                } else if (background) {
                    SDL_RenderCopy(renderer, background, NULL, NULL);
                }
                
                // Render game
                game.render(renderer, font);
                break;
                
            case GameState::GUIDE:
                // Draw background
                if (background) {
                    SDL_RenderCopy(renderer, background, NULL, NULL);
                }
                
                // Draw guide text
                if (font) {
                    SDL_Color textColor = {255, 255, 255, 255};
                    const char* lines[] = {
                        "HOW TO PLAY:",
                        "- Move your mouse to control the ball",
                        "- Avoid the obstacles coming from above",
                        "- Each obstacle you pass gives you 1 point",
                        "- The game gets faster as you score more",
                        "",
                        "Click to return to menu"
                    };
                    
                    int y = 100;
                    for (const char* line : lines) {
                        SDL_Surface* textSurface = TTF_RenderText_Solid(font, line, textColor);
                        if (textSurface) {
                            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                            if (textTexture) {
                                int x = (SCREEN_WIDTH - textSurface->w) / 2;
                                SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
                                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                                SDL_DestroyTexture(textTexture);
                                y += textSurface->h + 10;
                            }
                            SDL_FreeSurface(textSurface);
                        }
                    }
                }
                break;
                
            case GameState::SETTINGS:
                // Draw background
                if (background) {
                    SDL_RenderCopy(renderer, background, NULL, NULL);
                }
                
                // Draw settings text
                if (font) {
                    SDL_Color textColor = {255, 255, 255, 255};
                    const char* lines[] = {
                        "SETTINGS",
                        "Music Volume: [TODO]",
                        "Sound Effects: [TODO]",
                        "",
                        "Click to return to menu"
                    };
                    
                    int y = 100;
                    for (const char* line : lines) {
                        SDL_Surface* textSurface = TTF_RenderText_Solid(font, line, textColor);
                        if (textSurface) {
                            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                            if (textTexture) {
                                int x = (SCREEN_WIDTH - textSurface->w) / 2;
                                SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
                                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                                SDL_DestroyTexture(textTexture);
                                y += textSurface->h + 10;
                            }
                            SDL_FreeSurface(textSurface);
                        }
                    }
                }
                break;
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    if (background) SDL_DestroyTexture(background);
    if (gameBackground) SDL_DestroyTexture(gameBackground);
    if (font) TTF_CloseFont(font);
    if (titleFont) TTF_CloseFont(titleFont);
    if (selectFont) TTF_CloseFont(selectFont);
    if (bgMusic) Mix_FreeMusic(bgMusic);
    if (buttonSound) Mix_FreeChunk(buttonSound);
    if (crashSound) Mix_FreeChunk(crashSound);
    if (scoreSound) Mix_FreeChunk(scoreSound);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}