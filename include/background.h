#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <SDL.h>
#include "constants.h"

class Background {
    public:
        Background();
        void setTexture(SDL_Texture* tex);
        void setScrollSpeed(float speed);
        void update(float deltaTime);
        void render(SDL_Renderer* renderer);
        void reset();

    private:
        SDL_Texture* texture;   // Con trỏ tới texture của background
        float scrollY;          // Vị trí cuộn Y hiện tại
        float scrollSpeed;      // Tốc độ cuộn (pixels/giây)
        int textureWidth;       // Chiều rộng thực của texture
        int textureHeight;      // Chiều cao thực của texture
};
#endif
