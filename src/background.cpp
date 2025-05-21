#include "background.h"
#include <iostream>

Background::Background()
    : texture(nullptr), scrollY(0.0f), scrollSpeed(50.0f), textureWidth(0), textureHeight(0) {}

void Background::setTexture(SDL_Texture* tex) {
    texture = tex;
    if (texture) {
        // Lấy kích thước thực của texture
        SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);
    }
    scrollY = 0.0f; // Reset vị trí cuộn mỗi khi có texture mới
}
void Background::setScrollSpeed(float speed) {
    scrollSpeed = speed;
}
void Background::update(float deltaTime) {
    scrollY += scrollSpeed * deltaTime;

    while (scrollY >= static_cast<float>(textureHeight)) {
        scrollY -= static_cast<float>(textureHeight);
    }
}
void Background::render(SDL_Renderer* renderer) {
    SDL_Rect destRect1 = {0, static_cast<int>(scrollY), SCREEN_WIDTH, textureHeight};
    SDL_RenderCopy(renderer, texture, NULL, &destRect1);

    SDL_Rect destRect2 = {0, static_cast<int>(scrollY) - textureHeight, SCREEN_WIDTH, textureHeight};
    SDL_RenderCopy(renderer, texture, NULL, &destRect2);
}
void Background::reset() {
    scrollY = 0.0f;
}

