// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL.h>

// Kích thước màn hình
const int SCREEN_WIDTH = 448;
const int SCREEN_HEIGHT = 750;

// Giới hạn game
const int MAX_OBSTACLES = 8;
const int MAX_SPEED = 25;
const int OBSTACLE_SIZE = 40;

enum class GameState { 
    MENU, 
    PLAYING, 
    GUIDE, 
    SETTINGS ,
    PAUSED,
    VICTORY
};

#endif // CONSTANTS_H