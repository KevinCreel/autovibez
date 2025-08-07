#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

#include "mix_metadata.hpp"

namespace AutoVibez {
namespace UI {

class HelpOverlay {
public:
    HelpOverlay();
    ~HelpOverlay();

    void init(SDL_Window* window, SDL_GLContext glContext);
    void render();
    void toggle();
    bool isVisible() const {
        return _visible;
    }
    void setCursorVisibility(bool visible);
    void setFullscreenState(bool isFullscreen);
    void rebuildFontAtlas();              // Add method to rebuild font atlas
    void reinitializeImGui();             // Add method to completely reinitialize ImGui
    void triggerTextureRebind();          // Add method to trigger texture rebinding
    void triggerDeferredTextureRebind();  // Add method for deferred texture rebinding

    // Dynamic information methods
    void setCurrentPreset(const std::string& preset);
    void setCurrentMix(const std::string& artist, const std::string& title, const std::string& genre);
    void setVolumeLevel(int volume);
    void setAudioDevice(const std::string& device);
    void setBeatSensitivity(float sensitivity);

    // Mix table methods
    void setMixTableData(const std::vector<AutoVibez::Data::Mix>& mixes);
    void toggleMixTableFilter();

private:
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    bool _visible = false;
    bool _cursorWasVisible = true;
    bool _isFullscreen = false;
    bool _initialized = false;
    bool _imguiReady = false;
    bool _needsTextureRebind = false;          // Add flag for texture rebinding
    bool _needsDeferredTextureRebind = false;  // Add flag for deferred texture rebinding
    SDL_Cursor* _originalCursor = nullptr;
    SDL_Cursor* _blankCursor = nullptr;

    // Dynamic information
    std::string _currentPreset;
    std::string _currentArtist;
    std::string _currentTitle;
    std::string _currentGenre;
    int _volumeLevel = -1;
    std::string _audioDevice;
    float _beatSensitivity = 0.0f;

    // Mix table data
    std::vector<AutoVibez::Data::Mix> _mixTableData;
    bool _showFavoritesOnly = false;  // Add filter state

    // Alternative rendering
    TTF_Font* _font = nullptr;
    bool _useNativeRendering = false;

    // Key binding alignment helper
    struct KeyBinding {
        std::string key;
        std::string description;
    };

    float calculateMaxKeyWidth(const std::vector<KeyBinding>& bindings);
    void renderKeyBindingAligned(const std::string& key, const std::string& description, float maxWidth);
    void renderKeyBindingSection(const std::string& sectionTitle, const std::vector<KeyBinding>& bindings,
                                 const ImVec4& titleColor, const ImVec4& separatorColor);
    void renderStatusLabel(const std::string& label, const std::string& value, const ImVec4& valueColor,
                           float maxLabelWidth);
    void initializeImGui();
    void rebuildFontAtlasInternal();
};

}  // namespace UI
}  // namespace AutoVibez