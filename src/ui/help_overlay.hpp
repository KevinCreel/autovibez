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
    
    // Dynamic information methods
    void setCurrentPreset(const std::string& preset);
    void setCurrentMix(const std::string& artist, const std::string& title, const std::string& genre);
    void setVolumeLevel(int volume);
    void setAudioDevice(const std::string& device);
    void setBeatSensitivity(float sensitivity);

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
    
    // Dynamic information
    std::string _currentPreset;
    std::string _currentArtist;
    std::string _currentTitle;
    std::string _currentGenre;
    int _volumeLevel = 0;
    std::string _audioDevice;
    float _beatSensitivity = 1.0f;
    
    // Alternative rendering
    TTF_Font* _font = nullptr;
    bool _useNativeRendering = false;
};

} // namespace UI
} // namespace AutoVibez 