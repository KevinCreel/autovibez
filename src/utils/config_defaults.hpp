#pragma once

struct ConfigDefaults {
    // Cache settings
    static constexpr int DEFAULT_CACHE_SIZE_MB = 5120; // 5GB
    
    // Audio settings
    static constexpr int DEFAULT_SEEK_INCREMENT = 60;   // 60 seconds
    static constexpr int DEFAULT_VOLUME_STEP = 10;      // 10%
    static constexpr int DEFAULT_AUDIO_DEVICE = 0;
    
    // Crossfade settings
    static constexpr int DEFAULT_CROSSFADE_DURATION = 3000; // 3 seconds
    static constexpr bool DEFAULT_CROSSFADE_ENABLED = true;
    
    // Download settings
    static constexpr bool DEFAULT_AUTO_DOWNLOAD = true;
    
    // Display settings
    static constexpr bool DEFAULT_SHOW_FPS = false;
    static constexpr bool DEFAULT_FULLSCREEN = true;
    
    // Beat sensitivity settings
    static constexpr float DEFAULT_BEAT_SENSITIVITY = 1.0f;
    static constexpr float DEFAULT_HARD_CUT_SENSITIVITY = 1.0f;
    static constexpr bool DEFAULT_HARD_CUTS_ENABLED = false;
}; 