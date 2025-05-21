#include "character_selector.h"
#include <SDL_image.h>
#include <iostream>

bool CharacterSelector::loadResources(SDL_Renderer* renderer, const std::vector<std::string>& paths, const std::string& soundPath) {
    selectSound = Mix_LoadWAV(soundPath.c_str());
    if (!selectSound) {
        std::cerr << "Failed to load select sound: " << Mix_GetError() << std::endl;
    }

    // Load character textures
    for (const auto& path : paths) {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
            continue;
        }
        
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        
        if (texture) {
            characterTextures.push_back(texture);
            characterPaths.push_back(path);
        }
    }

    // Khởi tạo nhân vật hiệp sĩ với các trang phục
    std::vector<std::string> knightCostumes = {
        "assets/images/characters/Elf.png",
        "assets/images/characters/Wizart.png",
        "assets/images/characters/knight.png"
    };
    character.loadCostumes(renderer, knightCostumes);
    character.setPosition(characterRect.x, characterRect.y);
    character.setSize(characterRect.w, characterRect.h);

    return !characterTextures.empty();
}

void CharacterSelector::handleEvent(SDL_Event* e) {
    if (e->type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        if (x >= leftArrowRect.x && x <= leftArrowRect.x + leftArrowRect.w &&
            y >= leftArrowRect.y && y <= leftArrowRect.y + leftArrowRect.h) {
            selectedIndex = (selectedIndex - 1 + characterTextures.size()) % characterTextures.size();
            character.prevCostume(); // Chuyển trang phục trước đó
            if (selectSound) Mix_PlayChannel(-1, selectSound, 0);
        }
        else if (x >= rightArrowRect.x && x <= rightArrowRect.x + rightArrowRect.w &&
                 y >= rightArrowRect.y && y <= rightArrowRect.y + rightArrowRect.h) {
            selectedIndex = (selectedIndex + 1) % characterTextures.size();
            character.nextCostume(); // Chuyển trang phục tiếp theo
            if (selectSound) Mix_PlayChannel(-1, selectSound, 0);
        }
    }
}

void CharacterSelector::render(SDL_Renderer* renderer, TTF_Font* font) {
    // Thay thế render cũ bằng renderCharacterPreview
    renderCharacterPreview(renderer, font);
    
    // Vẫn giữ phần vẽ mũi tên và text
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

    // Draw "Select Character" text
    if (font) {
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Select Character", textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                int textX = (SCREEN_WIDTH - textSurface->w) / 2;
                int textY = characterRect.y + characterRect.h + 20;
                SDL_Rect textRect = {textX, textY, textSurface->w, textSurface->h};
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }
}
const std::string CHARACTER_NAMES[] = {
    "Elf",    // Tương ứng với selectedIndex = 0
    "Wizart", // Tương ứng với selectedIndex = 1
    "Knight"  // Tương ứng với selectedIndex = 2
};
void CharacterSelector::renderCharacterPreview(SDL_Renderer* renderer, TTF_Font* font) {
    // Cập nhật animation
    static Uint32 lastTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime ;
    if (lastTime == 0) {
        deltaTime = 1.0f / 60.0f;
    }
    else {
        deltaTime = (currentTime - lastTime) / 1000.0f;
    }
    const float MAX_ACCEPTABLE_DELTA_TIME = 0.25f;
    if (deltaTime > MAX_ACCEPTABLE_DELTA_TIME) {
        deltaTime = 1.0f / 60.0f;
    }
    if (deltaTime < 0.0f) {
        deltaTime = 0.0f;
    }
    character.update(deltaTime);
    lastTime = currentTime;

    // Vẽ nhân vật hiệp sĩ với animation
    character.render(renderer);
    
    // Vẽ tên trang phục
    SDL_Color textColor = {255, 255, 0, 255}; // Màu vàng
    std::string costumeName =CHARACTER_NAMES[character.getCurrentCostume()];
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, costumeName.c_str(), textColor);
    
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture) {
            SDL_Rect textRect = {
                characterRect.x + (characterRect.w - textSurface->w)/2,
                characterRect.y + characterRect.h + 40, // Đặt dưới text "Select Character"
                textSurface->w,
                textSurface->h
            };
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
}

std::string CharacterSelector::getSelectedCharacterPath() const {
    return characterPaths.empty() ? "" : characterPaths[selectedIndex];
}

CharacterSelector::~CharacterSelector() {
    for (auto texture : characterTextures) {
        SDL_DestroyTexture(texture);
    }
    if (selectSound) Mix_FreeChunk(selectSound);
}