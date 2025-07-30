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
    void rebuildFontAtlas(); // Add method to rebuild font atlas
    void reinitializeImGui(); // Add method to completely reinitialize ImGui
    void triggerTextureRebind(); // Add method to trigger texture rebinding
    void triggerDeferredTextureRebind(); // Add method for deferred texture rebinding
    void triggerCompleteReinitialization(); // Add method for complete reinitialization

private:
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    bool _visible = false;
    bool _cursorWasVisible = true;
    bool _isFullscreen = false;
    bool _initialized = false;
    bool _imguiReady = false;
    bool _needsTextureRebind = false; // Add flag for texture rebinding
    bool _needsDeferredTextureRebind = false; // Add flag for deferred texture rebinding
    SDL_Cursor* _originalCursor = nullptr;
    SDL_Cursor* _blankCursor = nullptr;
    
    // Alternative rendering
    TTF_Font* _font = nullptr;
    bool _useNativeRendering = false;
};

} // namespace UI
} // namespace AutoVibez 