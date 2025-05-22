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
#include "constants.h"
#include "character.h"
#include "character_selector.h"
#include "background.h"
#include "obstacle.h"

struct Button {
    SDL_Rect rect;
    SDL_Color color;
    const char* text;
};
SDL_Rect CreateCenteredHitbox(const SDL_Rect& originalRect, int hitboxWidth, int hitboxHeight) {
    SDL_Rect hitbox;
    hitbox.w = hitboxWidth;
    hitbox.h = hitboxHeight;
    hitbox.x = originalRect.x + (originalRect.w - hitbox.w) / 2;
    hitbox.y = originalRect.y + (originalRect.h - hitbox.h) / 2;
    return hitbox;
}
class Game {
private:
    ObstacleManager m_obstacleManager;
    Character character ;
    int score;
    bool isGameOver;
    Mix_Chunk* crashSound;
    Mix_Chunk* scoreSound;
    int baseSpeed;
    Background scrollingGameBackground;
    bool isVictory;
public:
    Game() : score(0), isGameOver(false), baseSpeed(2),isVictory(false) {}
    
    void init(SDL_Renderer* renderer, const std::string& characterPath, 
              const std::string& crashSoundPath, const std::string& scoreSoundPath,SDL_Texture* bgTex) {
        // Load character texture
        isGameOver = false;
        std::vector<std::string> costumePaths = {characterPath};
        character.loadCostumes(renderer, costumePaths);

        // Set initial character position
        character.setPosition(SCREEN_WIDTH/2 - 25, SCREEN_HEIGHT - 100);
        //character.setSize(50, 50);
        // Load sounds

        crashSound = Mix_LoadWAV(crashSoundPath.c_str());
        scoreSound = Mix_LoadWAV(scoreSoundPath.c_str());

        m_obstacleManager.loadTextures(renderer);
    
        if (bgTex) {
        scrollingGameBackground.setTexture(bgTex);
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
        int x = e->motion.x - character.getRect().w/2;
        x = std::max(0, std::min(x, SCREEN_WIDTH - character.getRect().w));
        int y= e->motion.y - character.getRect().h/2;
        y = std::max(0, std::min(y, SCREEN_HEIGHT - character.getRect().h));
        character.setPosition(x,y);
    }
}
    void update(float deltaTime) {
        if (isGameOver||isVictory) return;

        character.update(deltaTime);

        scrollingGameBackground.update(deltaTime);
        
        m_obstacleManager.setBaseSpeedFactor(this->baseSpeed);

        m_obstacleManager.update(deltaTime, this->score, this->scoreSound);

        if (this->score >= 500) { // Điều kiện thắng là 500 điểm
            this->isVictory = true;
            return;
        }
        SDL_Rect originalCharRect = character.getRect();
        const int CHARACTER_HITBOX_WIDTH = 30; 
        const int CHARACTER_HITBOX_HEIGHT = 30;
        SDL_Rect charCustomHitbox = CreateCenteredHitbox(originalCharRect, CHARACTER_HITBOX_WIDTH, CHARACTER_HITBOX_HEIGHT);
        for (const auto& obstacle : m_obstacleManager.getObstacles()) { // Lấy vật cản từ manager
            if (SDL_HasIntersection(&charCustomHitbox, &obstacle.rect)) {
                isGameOver = true;
                if (crashSound) Mix_PlayChannel(-1, crashSound, 0);
                return;
            }
        }
        
        if (score % 10 == 0 && score > 0) {
            baseSpeed = std::min(2 + score/10, MAX_SPEED);
        }
    }
    
    void render(SDL_Renderer* renderer, TTF_Font* font) {

        scrollingGameBackground.render(renderer);

        // Draw character
        character.render(renderer);
        
        // Draw obstacles
        m_obstacleManager.render(renderer);
        
        // Draw score
        if (font) {
            std::string scoreText = "Score: " + std::to_string(score);
            SDL_Color textColor = {255, 255, 255, 255};
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = {30, 30, textSurface->w, textSurface->h};
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }
        
        // Game over screen
        if (isGameOver) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderFillRect(renderer, &overlay);
            
            if (font) {
                SDL_Color textColor = {255, 255, 255, 255};
                
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
        score = 0;
        isGameOver = false;
        isVictory = false;
        baseSpeed = 2;
        scrollingGameBackground.reset();
        m_obstacleManager.reset();
    }
    
    bool gameOver() const { return isGameOver;}
    bool hasWon() const { return isVictory; }
    
    ~Game() {
        if (crashSound) Mix_FreeChunk(crashSound);
        if (scoreSound) Mix_FreeChunk(scoreSound);
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

    if (font && button.text) {
        SDL_Color textColor = { 255, 255, 255, 255 };
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, button.text, textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                int textX = button.rect.x + (button.rect.w - textSurface->w) / 2-25;
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

    Mix_Music* bgMusic = Mix_LoadMUS("assets/sounds/background.mp3");
    Mix_Chunk* buttonSound = Mix_LoadWAV("assets/sounds/button.mp3");
    Mix_Chunk* crashSound = Mix_LoadWAV("assets/sounds/crash.mp3");
    Mix_Chunk* scoreSound = Mix_LoadWAV("assets/sounds/score.mp3");
    
    if (bgMusic) {
        Mix_PlayMusic(bgMusic, -1);
        Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
    }

    SDL_Window* window = SDL_CreateWindow(
        "Game Vjpp",
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

    TTF_Font* font = TTF_OpenFont("assets/fonts/1.ttf", 50);
    TTF_Font* titleFont = TTF_OpenFont("assets/fonts/1.ttf", 100);
    TTF_Font* selectFont = TTF_OpenFont("assets/fonts/1.ttf", 30);
    if (!font || !titleFont || !selectFont) {
        std::cerr << "Failed to load fonts: " << TTF_GetError() << std::endl;
    }

    SDL_Texture* background = LoadTexture("assets/images/background.jpg", renderer);
    SDL_Texture* gameBackground = LoadTexture("assets/images/game_background.jpg", renderer);

    CharacterSelector characterSelector;
   if (!characterSelector.loadResources(renderer, 
    {"assets/images/characters/Elf.png",
     "assets/images/characters/Wizart.png", 
     "assets/images/characters/Knight.png"},
    "assets/sounds/select.mp3")) {
    std::cerr << "Failed to load character resources!" << std::endl;
        }

    Button buttons[3] = {
        {{150, 450, 200, 50}, {0, 255, 0, 255}, "Play"},
        {{150, 520, 200, 50}, {255, 165, 0, 255}, "Guide"},
        {{150, 590, 200, 50}, {255, 0, 0, 255}, "Settings"}
    };

    Button settingsMusicToggleButton;
    Button settingsBackButton = {{(SCREEN_WIDTH - 250) / 2, 300, 250, 50}, {100, 100, 100, 255}, "Back to Menu"};
    char musicToggleButtonText[50];

    const int PAUSE_BUTTON_WIDTH = 280;  
    const int PAUSE_BUTTON_HEIGHT = 50;  
    const int PAUSE_BUTTON_X_POS = (SCREEN_WIDTH - PAUSE_BUTTON_WIDTH) / 2; // Căn giữa theo chiều ngang
    const int PAUSE_BUTTON_SPACING = 40;  // Khoảng cách dọc giữa các nút
    int pause_buttons_start_y = SCREEN_HEIGHT / 2 - 50;

    Button pauseMenuButtons[3];
    pauseMenuButtons[0].rect = {PAUSE_BUTTON_X_POS,pause_buttons_start_y,PAUSE_BUTTON_WIDTH,PAUSE_BUTTON_HEIGHT};
    pauseMenuButtons[0].text = "    CONTINUE";
    pauseMenuButtons[0].color = {50, 200, 50, 255};

    pauseMenuButtons[1].rect = {PAUSE_BUTTON_X_POS,pause_buttons_start_y + PAUSE_BUTTON_HEIGHT + PAUSE_BUTTON_SPACING,PAUSE_BUTTON_WIDTH,PAUSE_BUTTON_HEIGHT};
    pauseMenuButtons[1].text = "RESUME";
    pauseMenuButtons[1].color = {200, 200, 50, 255};
    
    pauseMenuButtons[2].rect = {PAUSE_BUTTON_X_POS,pause_buttons_start_y + (PAUSE_BUTTON_HEIGHT + PAUSE_BUTTON_SPACING) * 2,PAUSE_BUTTON_WIDTH,PAUSE_BUTTON_HEIGHT};
    pauseMenuButtons[2].text = "BACK TO Menu";
    pauseMenuButtons[2].color = {200, 50, 50, 255};

    GameState currentState = GameState::MENU;
    Game game;
    Uint32 lastFrameTime = SDL_GetTicks();
    bool isRunning = true;
    SDL_Event event;
    bool isMusicOn = true;
    int musicVolumeWhenOn = MIX_MAX_VOLUME / 2;

        if (bgMusic) {
            Mix_PlayMusic(bgMusic, -1);
            if (isMusicOn) {
                Mix_VolumeMusic(musicVolumeWhenOn);
            }
            else {
            Mix_VolumeMusic(0);
            }
        }
    std::vector<DialogueLine> victoryDialogueScript;
    int currentVictoryDialogueLine = 0;

    SDL_Texture* playerPortraitVictory = nullptr; 
    SDL_Texture* npcPortraitVictory = LoadTexture("assets/images/hdieu.png", renderer)  ;
    SDL_Texture* dialogueBoxBackground = nullptr;
    SDL_Texture* victoryStateBackground=LoadTexture("assets/images/victory.png",renderer);

    while (isRunning) {

        Uint32 currentFrameTime = SDL_GetTicks();
        float deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
            switch (currentState) {
                case GameState::MENU:
                    if (event.type == SDL_MOUSEBUTTONDOWN) {
                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        
                        characterSelector.handleEvent(&event);
                        
                        for (int i = 0; i < 3; i++) {
                            if (mouseX >= buttons[i].rect.x && mouseX <= buttons[i].rect.x + buttons[i].rect.w &&
                                mouseY >= buttons[i].rect.y && mouseY <= buttons[i].rect.y + buttons[i].rect.h) {
                                if (buttonSound) Mix_PlayChannel(-1, buttonSound, 0);
                                
                                if (i == 0) {
                                    game.init(renderer, characterSelector.getSelectedCharacterPath(),
                                              "assets/sounds/crash.mp3", "assets/sounds/score.mp3",gameBackground);
                                    game.reset();
                                    currentState = GameState::PLAYING;
                                } else if (i == 1) {
                                    currentState = GameState::GUIDE;
                                } else if (i == 2) {
                                    currentState = GameState::SETTINGS;
                                }
                            }
                        }
                    }
                    break;
                    
                case GameState::PLAYING:
                    if (event.type == SDL_KEYDOWN) {
                    // Nhấn 'P' hoặc 'Escape' để Pause game
                        if (event.key.keysym.sym == SDLK_p || event.key.keysym.sym == SDLK_ESCAPE) {
                            currentState = GameState::PAUSED;
                        }
                    }
                    if (game.gameOver()) {
                        if (event.type == SDL_MOUSEBUTTONDOWN) {
                            currentState = GameState::MENU;
                        }
                    } else {
                        game.handleEvent(&event);
                    }
                    break;
                case GameState::PAUSED:
                    if (event.type == SDL_MOUSEBUTTONDOWN) {
                        int mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        SDL_Point clickPoint = {mouseX, mouseY};
                        
                        if (SDL_PointInRect(&clickPoint, &pauseMenuButtons[0].rect)) {
                            currentState = GameState::PLAYING;
                        }
                        
                        else if (SDL_PointInRect(&clickPoint, &pauseMenuButtons[1].rect)) {
                            game.reset(); // Reset lại trò chơi
                            currentState = GameState::PLAYING; // Chuyển sang trạng thái chơi
                        }
                        
                        else if (SDL_PointInRect(&clickPoint, &pauseMenuButtons[2].rect)) {
                            currentState = GameState::MENU;
                        }
                    }
                break;

                case GameState::GUIDE:
                case GameState::SETTINGS:
                if (event.type == SDL_MOUSEBUTTONDOWN) {

                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    SDL_Point clickPoint = {mouseX, mouseY};

                    if (currentState == GameState::SETTINGS) {

                        if (SDL_PointInRect(&clickPoint, &settingsMusicToggleButton.rect)) {
                            isMusicOn = !isMusicOn;
                            if (isMusicOn) { Mix_VolumeMusic(musicVolumeWhenOn); }
                            else { Mix_VolumeMusic(0); }
                            if (buttonSound) Mix_PlayChannel(-1, buttonSound, 0);
                        } else if (SDL_PointInRect(&clickPoint, &settingsBackButton.rect)) {
                            currentState = GameState::MENU; // Nút Back trong Settings đưa về MENU
                            if (buttonSound) Mix_PlayChannel(-1, buttonSound, 0);
                        }

                    } else {
                        currentState = GameState::MENU;
                        if (buttonSound) Mix_PlayChannel(-1, buttonSound, 0);
                    }
                }
                break;

                case GameState::VICTORY:
                if (event.type == SDL_MOUSEBUTTONDOWN || 
                    (event.type == SDL_KEYDOWN && event.key.repeat == 0)) {
                    currentVictoryDialogueLine++;
                    if (buttonSound) Mix_PlayChannel(-1, buttonSound, 0);

                    if (static_cast<size_t>(currentVictoryDialogueLine) >= victoryDialogueScript.size()) {
                        currentState = GameState::MENU;
                        game.reset();
                        if (isMusicOn && bgMusic) {
                             if(Mix_PlayingMusic() == 0 || Mix_PausedMusic() == 1) Mix_PlayMusic(bgMusic, -1);
                             Mix_VolumeMusic(musicVolumeWhenOn);
                        } else if (!isMusicOn) {
                             Mix_VolumeMusic(0);
                        }
                    }
                }
                break;
            }
        }

        if (currentState == GameState::PLAYING && !game.gameOver()&& !game.hasWon()) {
            game.update(deltaTime);
        }
        if (currentState == GameState::PLAYING && game.hasWon()) {
            currentState = GameState::VICTORY;
            currentVictoryDialogueLine = 0;
            if (bgMusic && Mix_PlayingMusic()) { // Chỉ dừng nếu nhạc đang phát
                Mix_HaltMusic();
            }
            victoryDialogueScript.clear();
            victoryDialogueScript.push_back({"...", "YOU WIN"}); 
           // victoryDialogueScript.push_back({" H Dieu", ""}); 
            //victoryDialogueScript.push_back({"", ""});
        }

        SDL_RenderClear(renderer);

        switch (currentState) {
            case GameState::MENU:
                if (background) {
                    SDL_RenderCopy(renderer, background, NULL, NULL);
                }

                if (titleFont) {
                    SDL_Color titleColor = {255, 255, 255, 255};
                    SDL_Surface* titleSurface = TTF_RenderText_Solid(titleFont, "GAME VIPP", titleColor);
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
                
                characterSelector.render(renderer, selectFont);
                
                for (const auto& button : buttons) {
                    DrawButton(renderer, button, font);
                }
                break;
                
            case GameState::PLAYING:
            
                game.render(renderer, font);
                break;
            case GameState::PAUSED: {
                game.render(renderer, font); 
                //Lớp Phủ
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Bật chế độ trộn màu
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Màu đen, alpha 180 (độ mờ ~70%)
                SDL_Rect overlayRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_RenderFillRect(renderer, &overlayRect);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // Tắt chế độ trộn màu  
                if (titleFont) {
                    SDL_Surface* surf = TTF_RenderText_Solid(titleFont, "PAUSING", {255, 255, 255, 255});
                    if (surf) {
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);

                    int title_y = pauseMenuButtons[0].rect.y - surf->h - 40; // cách nút đầu tiên 40px
        
                    SDL_Rect titleRect = {(SCREEN_WIDTH - surf->w) / 2, title_y, surf->w, surf->h};
                    SDL_RenderCopy(renderer, tex, NULL, &titleRect);
                    SDL_DestroyTexture(tex);
                    SDL_FreeSurface(surf);
                    }
                }
                for (int i = 0; i < 3; ++i) {
                    DrawButton(renderer, pauseMenuButtons[i], font);
                }
                break;
            }
            case GameState::GUIDE:
                if (background) {
                    SDL_RenderCopy(renderer, background, NULL, NULL);
                }
                
                if (selectFont) {
                    SDL_Color textColor = {255, 255, 255, 255};
                    const char* lines[] = {
                        "HOW TO PLAY:",
                        "- Move your mouse to control the character",
                        "- Avoid the obstacles coming from above",
                        "- Each obstacle you pass gives you 1 point",
                        "- The game gets faster as you score more",
                        "- Press P or ESC to pause game",
                        "- You win when you reach 1000 points",
                        "",
                        "Click to return to menu"
                    };
                    
                    int y = 100;
                    for (const char* line : lines) {
                        SDL_Surface* textSurface = TTF_RenderText_Solid(selectFont, line, textColor);
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
                if (background) {
                    SDL_RenderCopy(renderer, background, NULL, NULL);
                }
                
                if (selectFont) {
                    SDL_Color textColor = {255, 255, 255, 255};
                    
                    int y = 100;
                    SDL_Surface* surf = TTF_RenderText_Solid(titleFont, "SETTINGS", textColor);
                    if (surf) {
                        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                        SDL_Rect dst = {(SCREEN_WIDTH - surf->w) / 2, y, surf->w, surf->h};
                        SDL_RenderCopy(renderer, tex, NULL, &dst);
                        y += surf->h + 40; // Tăng khoảng cách Y
                        SDL_FreeSurface(surf);
                        SDL_DestroyTexture(tex);
                    }
                    sprintf(musicToggleButtonText, "Music: [ %s ]", (isMusicOn ? "ON" : "OFF"));
                    settingsMusicToggleButton.text = musicToggleButtonText;
                    settingsMusicToggleButton.rect = {(SCREEN_WIDTH - 250) / 2, y, 250, 50};
                    DrawButton(renderer, settingsMusicToggleButton, font);
                    settingsBackButton.rect.x = (SCREEN_WIDTH - settingsBackButton.rect.w) / 2; // Căn giữa X
                    settingsBackButton.rect.y = y+100; // Đặt ở vị trí Y mới
                    DrawButton(renderer, settingsBackButton, font); // Vẽ nút Back
                }
                break;
            case GameState::VICTORY: { 
            if (victoryStateBackground) { 
                SDL_RenderCopy(renderer, victoryStateBackground, NULL, NULL);
            } 
            // Vẽ hộp thoại
            SDL_Rect dialogueBoxRect = { SCREEN_WIDTH / 10, SCREEN_HEIGHT * 2 / 3 - 20, SCREEN_WIDTH * 8 / 10, SCREEN_HEIGHT / 3 };
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 10, 10, 30, 220);
            SDL_RenderFillRect(renderer, &dialogueBoxRect);
            SDL_SetRenderDrawColor(renderer, 180, 180, 220, 255);
            SDL_RenderDrawRect(renderer, &dialogueBoxRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

             if (npcPortraitVictory && static_cast<size_t>(currentVictoryDialogueLine) < victoryDialogueScript.size()) {
                 const DialogueLine& currentLine = victoryDialogueScript[currentVictoryDialogueLine];
                 if (currentLine.speakerName == "PRINCESS") { 
                      SDL_Rect portraitDestRect = {dialogueBoxRect.x + dialogueBoxRect.w - 150 - 10, dialogueBoxRect.y - 160, 150, 150};
                      SDL_RenderCopy(renderer, npcPortraitVictory, NULL, &portraitDestRect);
                 }
             }


            if (font && static_cast<size_t>(currentVictoryDialogueLine) < victoryDialogueScript.size()) {
                const DialogueLine& currentLine = victoryDialogueScript[currentVictoryDialogueLine];
                SDL_Color speakerNameColor = {255, 223, 0, 255}; 
                SDL_Color dialogueTextColor = {255, 255, 255, 255};    

                int textX = dialogueBoxRect.x + 20;
                int currentTextY = dialogueBoxRect.y + 20;

                if (!currentLine.speakerName.empty()) {
                    std::string speakerText = currentLine.speakerName + ":";
                    SDL_Surface* speakerSurf = TTF_RenderText_Solid(font, speakerText.c_str(), speakerNameColor);
                    if (speakerSurf) {
                        SDL_Texture* speakerTex = SDL_CreateTextureFromSurface(renderer, speakerSurf);
                        SDL_Rect speakerRect = {textX, currentTextY, speakerSurf->w, speakerSurf->h};
                        SDL_RenderCopy(renderer, speakerTex, NULL, &speakerRect);
                        SDL_DestroyTexture(speakerTex);
                        SDL_FreeSurface(speakerSurf);
                        currentTextY += speakerRect.h + 8; 
                    }
                }

                SDL_Surface* lineSurf = TTF_RenderText_Solid_Wrapped(font, currentLine.text.c_str(), dialogueTextColor, dialogueBoxRect.w - 40);
                if (lineSurf) {
                    SDL_Texture* lineTex = SDL_CreateTextureFromSurface(renderer, lineSurf);
                    SDL_Rect lineRect = {textX, currentTextY, lineSurf->w, lineSurf->h};
                    SDL_RenderCopy(renderer, lineTex, NULL, &lineRect);
                    SDL_DestroyTexture(lineTex);
                    SDL_FreeSurface(lineSurf);
                }

                std::string promptText = "Nhan de tiep tuc...";
                SDL_Surface* promptSurf = TTF_RenderText_Solid(font, promptText.c_str(), {180, 180, 180, 255}); 
                if(promptSurf){
                    SDL_Texture* promptTex = SDL_CreateTextureFromSurface(renderer, promptSurf);
                    SDL_Rect promptDst = {
                        dialogueBoxRect.x + dialogueBoxRect.w - promptSurf->w - 15, 
                        dialogueBoxRect.y + dialogueBoxRect.h - promptSurf->h - 10, 
                        promptSurf->w, promptSurf->h
                    };
                    SDL_RenderCopy(renderer, promptTex, NULL, &promptDst);
                    SDL_DestroyTexture(promptTex);
                    SDL_FreeSurface(promptSurf);
                }

            } else if (font && static_cast<size_t>(currentVictoryDialogueLine) >= victoryDialogueScript.size()) {
                SDL_Surface* surf = TTF_RenderText_Solid(font, "Nhan de ve Menu", {255,255,255,255});
                if (surf) {
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                    SDL_Rect dst = {(SCREEN_WIDTH - surf->w) / 2, SCREEN_HEIGHT / 2, surf->w, surf->h};
                    SDL_RenderCopy(renderer, tex, NULL, &dst);
                    SDL_DestroyTexture(tex);
                    SDL_FreeSurface(surf);
                }
            }
            break;
        }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    if (victoryStateBackground) SDL_DestroyTexture(victoryStateBackground);
    if (npcPortraitVictory) SDL_DestroyTexture(npcPortraitVictory);
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