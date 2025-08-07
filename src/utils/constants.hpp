#pragma once

namespace Constants {
    // Audio
    constexpr int DEFAULT_SAMPLE_RATE = 44100;
    constexpr int DEFAULT_CHANNELS = 2;
    constexpr int DEFAULT_SAMPLES = 512;
    constexpr int DEFAULT_BUFFER_SIZE = 2048;
    constexpr int MAX_VOLUME = 100;
    constexpr int MIN_VOLUME = 0;
    
    // Beat sensitivity
    constexpr float MIN_BEAT_SENSITIVITY = 0.0f;
    constexpr float MAX_BEAT_SENSITIVITY = 5.0f;
    constexpr float BEAT_SENSITIVITY_STEP = 0.1f;
    
    // File validation
    constexpr int MIN_MP3_FILE_SIZE = 1024;  // Minimum valid MP3 file size in bytes
    
    // Crossfade
    constexpr int DEFAULT_CROSSFADE_DURATION_MS = 3000;
    constexpr int MAX_CROSSFADE_DURATION_MS = 10000;
    constexpr int MIN_CROSSFADE_DURATION_MS = 500;
    
    // Database
    constexpr int MAX_RETRIES = 3;
    constexpr int DEFAULT_TIMEOUT_SECONDS = 30;
    constexpr int MAX_DOWNLOAD_RETRIES = 5;
    
    // Download
    constexpr int MIN_DOWNLOAD_SPEED_BYTES_PER_SEC = 1000;  // 1KB/s minimum
    constexpr int DEFAULT_DOWNLOAD_TIMEOUT_SECONDS = 60;
    
    // UI/Display
    constexpr int DEFAULT_WINDOW_WIDTH = 1024;
    constexpr int DEFAULT_WINDOW_HEIGHT = 768;
    constexpr int DEFAULT_FPS = 60;
    constexpr int FRAME_DELAY_MS = 1000 / DEFAULT_FPS;  // ~16.67ms
    
    // Smart selection probabilities (percentages)
    constexpr int PREFERRED_GENRE_PROBABILITY = 80;  // 80% chance to prefer genre
    constexpr int FAVORITE_MIX_PROBABILITY = 70;     // 70% chance to prefer favorites
    
    // Test constants
    constexpr int TEST_ITERATIONS = 1000;
    constexpr int TEST_SLEEP_MS = 100;
    constexpr int TEST_CACHE_SIZE_MB = 100;
} 