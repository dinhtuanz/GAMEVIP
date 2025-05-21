// src/character.cpp
#include "character.h"
#include <SDL_image.h> // IMG_LoadTexture cần SDL_image.h (đã có trong character.h của bạn)
#include <iostream>    // Cho std::cerr, std::cout

// Hàm khởi tạo - Đã chính xác!
Character::Character() : 
    position({0, 0, 50, 50}),  // Kích thước frame mặc định
    currentCostume(0),
    currentFrame(0),
    frameTime(0.0f),           // Nên là 0.0f cho float
    animationSpeed(0.1f) {}

Character::~Character() {
    for (auto texture : costumes) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
    costumes.clear();
}

bool Character::loadCostumes(SDL_Renderer* renderer, const std::vector<std::string>& costumePaths) {
    for (auto texture : costumes) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
    costumes.clear();

    for (const auto& path : costumePaths) {
        SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
        if (!texture) {
            std::cerr << "Character::loadCostumes - Failed to load texture: " << path << " - " << IMG_GetError() << std::endl;
            continue;
        }
        
        int width, height;
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
        std::cout << "Character::loadCostumes - Loaded sprite sheet: " << path 
                  << " (" << width << "x" << height << ")" << std::endl;
        
        costumes.push_back(texture);
    }
    
    if (costumes.empty() && !costumePaths.empty()) {
         std::cerr << "Character::loadCostumes - Warning: No costumes loaded despite paths being provided." << std::endl;
    }
    return !costumes.empty();
}

void Character::setPosition(int x, int y) {
    position.x = x;
    position.y = y;
}

void Character::setSize(int w, int h) {
    position.w = w;
    position.h = h;
}

void Character::update(float deltaTime) {
    // Đảm bảo animationSpeed > 0 để tránh chia cho 0 nếu logic thay đổi
    if (animationSpeed <= 0.0f) return;

    frameTime += deltaTime;
    if (frameTime >= animationSpeed) {
        // frameTime = 0; // Reset đơn giản
        frameTime -= animationSpeed; // Giữ lại phần thời gian dư cho khung hình tiếp theo chính xác hơn
        if (frameTime < 0) frameTime = 0; // Đảm bảo không âm nếu deltaTime rất lớn

        currentFrame = (currentFrame + 1) % 3; // Giả sử sprite sheet có 3 frame (0, 1, 2)
    }
}

// << SỬA ĐỔI QUAN TRỌNG TRONG PHƯƠNG THỨC RENDER >>
void Character::render(SDL_Renderer* renderer) {

    if (!costumes.empty() && currentCostume >= 0 &&  static_cast<size_t>(currentCostume) < costumes.size() &&  costumes[static_cast<size_t>(currentCostume)] != nullptr) {       // Quan trọng: Kiểm tra texture không phải là nullptr

        const int FRAME_WIDTH = 50;  // Chiều rộng của một frame 
        const int FRAME_HEIGHT = 50; // Chiều cao của một frame
        int frameToRender = currentFrame % 3; 

        SDL_Rect srcRect;
        srcRect.x = frameToRender * FRAME_WIDTH; 
        srcRect.y = 0;                          
        srcRect.w = FRAME_WIDTH;                
        srcRect.h = FRAME_HEIGHT;               
        
        // Sử dụng static_cast cho currentCostume khi truy cập vector để nhất quán
        SDL_RenderCopy(renderer, costumes[static_cast<size_t>(currentCostume)], &srcRect, &position);
    }
}

void Character::nextCostume() {
    if (!costumes.empty()) {
        currentCostume = (currentCostume + 1) % static_cast<int>(costumes.size());
    }
}

void Character::prevCostume() {
    if (!costumes.empty()) {
        currentCostume = (currentCostume - 1 + static_cast<int>(costumes.size())) % static_cast<int>(costumes.size());
    }
}

SDL_Rect Character::getRect() const {
    return position;
}

int Character::getCurrentCostume() const {
    return currentCostume;
}