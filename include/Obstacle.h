#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <vector>
#include <string>
#include <random>
#include <SDL.h>
#include <SDL_mixer.h>
#include "constants.h"

struct Obstacle {
    SDL_Rect rect;
    float speed;    
    bool passed;
    int textureIndex;
};
class ObstacleManager {
public:
    ObstacleManager();
    ~ObstacleManager();

    void loadTextures(SDL_Renderer* renderer);
    void update(float deltaTime, int& currentScore, Mix_Chunk* scoreSoundEffect);
    void render(SDL_Renderer* renderer);
    void reset();

    const std::vector<Obstacle>& getObstacles() const;

    void setBaseSpeedFactor(int factor);

private:
    void spawnObstacles(int count);
    std::vector<Obstacle> m_obstacles;
    std::vector<SDL_Texture*> m_obstacleTextures;
    SDL_Renderer* m_renderer; // Lưu con trỏ renderer để load textures
    std::mt19937 m_rng;       // Bộ tạo số ngẫu nhiên riêng
    int m_baseSpeedFactor;    // Yếu tố tốc độ cơ bản, được Game cập nhật
};
#endif