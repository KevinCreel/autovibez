#pragma once

#include <string>
#include <SDL2/SDL.h>

/**
 * Centralized application state management
 */
class AppState {
public:
    /**
     * Window state management
     */
    struct WindowState {
        bool isFullscreen{false};
        bool isStretched{false};
        int width{512};
        int height{512};
        bool isResizable{true};
        bool allowHighDPI{true};
    };
    
    /**
     * Audio state management
     */
    struct AudioState {
        bool isCapturing{false};
        bool isWASAPI{false};
        bool isFakeAudio{false};
        int currentDevice{0};
        int deviceCount{0};
        int selectedDeviceIndex{0};
        unsigned short channelsCount{0};
        SDL_AudioDeviceID deviceId{0};
    };
    
    /**
     * UI state management
     */
    struct UIState {
        bool showHelp{false};
        bool showFps{false};
        bool mouseDown{false};
        bool keymod{false};
        bool volumeKeyPressed{false};
        bool manualPresetChange{false};
    };
    
    /**
     * Mix state management
     */
    struct MixState {
        bool managerInitialized{false};
        bool showMixStatus{false};
        int mixStatusDisplayTime{0};
        bool mixInfoDisplayed{false};
        std::string currentMixId;
        std::string currentGenre;
    };
    
    /**
     * Preset state management
     */
    struct PresetState {
        std::string currentPresetName;
        bool isPlaying{false};
        bool isPaused{false};
    };
    
    // State instances
    WindowState window;
    AudioState audio;
    UIState ui;
    MixState mix;
    PresetState preset;
    
    /**
     * Reset all state to defaults
     */
    void reset();
    
    /**
     * Get a summary of current state for debugging
     */
    std::string getStateSummary() const;
}; 