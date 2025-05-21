// character.h
#ifndef CHARACTER_H
#define CHARACTER_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <vector>
#include"constants.h"
#include<SDL_ttf.h>

class Character {
private:
    SDL_Rect position;
    std::vector<SDL_Texture*> animations; // Các frame animation
    std::vector<SDL_Texture*> costumes;   // Các trang phục
    int currentCostume;
    int currentFrame;
    float frameTime;
    float animationSpeed;
    
public:
    Character();
    ~Character();
    
    bool loadCostumes(SDL_Renderer* renderer, const std::vector<std::string>& costumePaths);
    void setPosition(int x, int y);
    void setSize(int w, int h);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void nextCostume();
    void prevCostume();
    
    SDL_Rect getRect() const;
    int getCurrentCostume() const;
};

#endif