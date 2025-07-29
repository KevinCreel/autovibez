#pragma once

#include <string>

namespace Constants {
    // Audio
    constexpr int DEFAULT_SAMPLE_RATE = 44100;
    constexpr int DEFAULT_CHANNELS = 2;
    constexpr int DEFAULT_SAMPLES = 512;
    
    // Beat sensitivity
    constexpr float MIN_BEAT_SENSITIVITY = 0.0f;
    constexpr float MAX_BEAT_SENSITIVITY = 5.0f;
    constexpr float BEAT_SENSITIVITY_STEP = 0.1f;
    
    // UI
    constexpr int DEFAULT_WINDOW_WIDTH = 512;
    constexpr int DEFAULT_WINDOW_HEIGHT = 512;
    constexpr int MIX_STATUS_DISPLAY_TIME = 300; // 5 seconds
    
    // PCM
    constexpr int PCM_BUFFER_SIZE = 512;
    constexpr int PCM_MAX_VALUE = 16384; // 2^14
    
    // Window
    constexpr int DEFAULT_FPS = 60;
    constexpr int FRAME_DELAY_MS = 1000 / DEFAULT_FPS;
    
    // File paths - These are fallbacks, actual paths are determined by PathManager
    constexpr const char* DEFAULT_CONFIG_FILE = "config/config.inp";
    constexpr const char* DEFAULT_PRESET_PATH = "assets/presets";
    constexpr const char* DEFAULT_TEXTURE_PATH = "assets/textures";
    
    // Application
    constexpr const char* APP_NAME = "AutoVibez";
    constexpr const char* WINDOW_TITLE = "AutoVibez";
} 