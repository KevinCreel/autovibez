#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <imgui.h>
#include <functional>
#include <vector>
#include <string>

namespace AutoVibez {
namespace UI {

class HelpOverlay {
public:
    HelpOverlay();
    ~HelpOverlay();
    
    void init(SDL_Window* window, SDL_GLContext glContext);
    void render();
    void toggle();
    bool isVisible() const { return _visible; }
    void setCursorVisibility(bool visible);
    void setFullscreenState(bool isFullscreen);

private:
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    bool _visible = false;
    bool _cursorWasVisible = true;
    bool _isFullscreen = false;
    bool _initialized = false;
    bool _imguiReady = false;
    SDL_Cursor* _originalCursor = nullptr;
    SDL_Cursor* _blankCursor = nullptr;
    
    // Alternative rendering
    TTF_Font* _font = nullptr;
    bool _useNativeRendering = false;
};

} // namespace UI
} // namespace AutoVibez 