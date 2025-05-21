#ifndef CHARACTER_SELECTOR_H
#define CHARACTER_SELECTOR_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include "constants.h"
#include "character.h"  // Thêm include này để sử dụng class Character

class CharacterSelector {
private:
    std::vector<SDL_Texture*> characterTextures;
    std::vector<std::string> characterPaths;
    int selectedIndex = 0;
    SDL_Rect characterRect = { (SCREEN_WIDTH - 100)/2, 250, 100, 100 };
    SDL_Rect leftArrowRect = { characterRect.x - 50, characterRect.y + (characterRect.h - 30)/2, 30, 30 };
    SDL_Rect rightArrowRect = { characterRect.x + characterRect.w + 20, characterRect.y + (characterRect.h - 30)/2, 30, 30 };
    Mix_Chunk* selectSound = nullptr;
    Character character;  // Thêm thành viên Character
    
public:
    bool loadResources(SDL_Renderer* renderer, const std::vector<std::string>& paths, const std::string& soundPath);
    void handleEvent(SDL_Event* e);
    void render(SDL_Renderer* renderer, TTF_Font* font);
    std::string getSelectedCharacterPath() const;
    ~CharacterSelector();
    
    // Khai báo hàm renderCharacterPreview (không định nghĩa trong header)
    void renderCharacterPreview(SDL_Renderer* renderer, TTF_Font* font);
};

#endif // CHARACTER_SELECTOR_H