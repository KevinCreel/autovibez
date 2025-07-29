#pragma once

#include <SDL2/SDL.h>
#include <imgui.h>
#include <functional>
#include <vector>
#include <string>

class SimpleUI {
public:
    SimpleUI();
    ~SimpleUI();
    
    void init(SDL_Window* window, SDL_GLContext glContext);
    void render();
    void toggle();
    bool isVisible() const { return _visible; }

private:
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    bool _visible = false;
}; 