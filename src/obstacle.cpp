#include "obstacle.h"
#include <SDL_image.h> 
#include <iostream>   
#include <algorithm>   // Cho std::remove_if
#include <ctime>
ObstacleManager::ObstacleManager() : m_renderer(nullptr), m_rng(std::time(nullptr)), m_baseSpeedFactor(2) {}

ObstacleManager::~ObstacleManager() {
    for (SDL_Texture* texture : m_obstacleTextures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }
    m_obstacleTextures.clear();
}

void ObstacleManager::loadTextures(SDL_Renderer* renderer) {
    m_renderer = renderer;

    for (SDL_Texture* tex : m_obstacleTextures) { if (tex) SDL_DestroyTexture(tex); }

    m_obstacleTextures.clear();
    for (int i = 1; i <= 7; i++) {
        std::string path = "assets/images/obstacles/" + std::to_string(i) + ".png";
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
            SDL_FreeSurface(surface);
            if (texture) {
                m_obstacleTextures.push_back(texture);
            }
        }
    }
}
void ObstacleManager::setBaseSpeedFactor(int factor) {
        m_baseSpeedFactor = std::max(1, factor); // Đảm bảo factor ít nhất là 1
    }

void ObstacleManager::spawnObstacles(int count) {

    if (m_obstacleTextures.empty() || std::all_of(m_obstacleTextures.begin(), m_obstacleTextures.end(), [](SDL_Texture* t){ return t == nullptr; })) {
        return;
    }
    int current_obstacle_count = m_obstacles.size();
    int obstaclesToCreate = std::min(count, MAX_OBSTACLES - current_obstacle_count);

    if (obstaclesToCreate <= 0) {
        return;
    }

    std::uniform_int_distribution<int> distX(0, SCREEN_WIDTH - OBSTACLE_SIZE);
    std::uniform_int_distribution<int> distSpeedBase(2, m_baseSpeedFactor);
    std::uniform_int_distribution<int> distTexture(0, m_obstacleTextures.size() - 1);

    for (int i = 0; i < obstaclesToCreate; ++i) {
        int speed_factor = distSpeedBase(m_rng);
        float speed_pps = static_cast<float>(speed_factor) * 60.0f;

        int spawn_y = -OBSTACLE_SIZE - (i * 150);
   
        m_obstacles.push_back({
            {distX(m_rng), spawn_y, OBSTACLE_SIZE, OBSTACLE_SIZE},
            speed_pps,
            false, // passed
            distTexture(m_rng)
        });
    }
}

void ObstacleManager::update(float deltaTime, int& currentScore, Mix_Chunk* scoreSoundEffect) {
    for (auto& obstacle : m_obstacles) {
        obstacle.rect.y += static_cast<int>(obstacle.speed * deltaTime);

        // Kiểm tra và tính điểm nếu vật cản đi qua màn hình
        if (!obstacle.passed && obstacle.rect.y > SCREEN_HEIGHT) {
            obstacle.passed = true;
            currentScore++; // ObstacleManager cập nhật điểm trực tiếp
            if (scoreSoundEffect) {
                Mix_PlayChannel(-1, scoreSoundEffect, 0);
            }
        }
    }

    // Xóa các vật cản đã đi qua và ra khỏi màn hình
    m_obstacles.erase(
        std::remove_if(m_obstacles.begin(), m_obstacles.end(),
                       [](const Obstacle& o) {
                           return o.passed && o.rect.y > SCREEN_HEIGHT + OBSTACLE_SIZE;
                       }),
        m_obstacles.end()
    );
    // Logic tạo vật cản mới (ví dụ: nếu số lượng ít hoặc vật cản cuối cùng đã di chuyển đủ xa)
    // Đây là một ví dụ đơn giản, bạn có thể cần tinh chỉnh điều kiện này
    if (m_obstacles.size() < MAX_OBSTACLES / 2 ||
        (!m_obstacles.empty() && m_obstacles.back().rect.y > -OBSTACLE_SIZE && m_obstacles.size() < MAX_OBSTACLES)) {
        spawnObstacles(1); // Tạo một vật cản mới. Khi count = 1, i = 0, nên spawn_y = -OBSTACLE_SIZE.
    }
}


void ObstacleManager::render(SDL_Renderer* renderer) {
    for (const auto& obstacle : m_obstacles) {
        if (static_cast<size_t>(obstacle.textureIndex) < m_obstacleTextures.size() && m_obstacleTextures[obstacle.textureIndex] != nullptr) {
            SDL_RenderCopy(renderer, m_obstacleTextures[obstacle.textureIndex], NULL, &obstacle.rect);
        }
    }
}

void ObstacleManager::reset() {
    m_obstacles.clear();
   spawnObstacles(3);
}

const std::vector<Obstacle>& ObstacleManager::getObstacles() const {
    return m_obstacles;
}
