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
    
    // File paths - These are fallbacks, actual paths are determined by PathManager
    constexpr const char* DEFAULT_CONFIG_FILE = "config/config.inp";
    constexpr const char* DEFAULT_PRESET_PATH = "assets/presets";
    constexpr const char* DEFAULT_TEXTURE_PATH = "assets/textures";
} 