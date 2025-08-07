#pragma once

#include <projectM-4/playlist.h>

#include <random>
#include <string>

class PresetManager {
public:
    explicit PresetManager(projectm_playlist_handle playlist);

    /**
     * Jump to random preset
     */
    void randomPreset();

private:
    projectm_playlist_handle _playlist;
    std::mt19937 _randomGenerator;  // Persistent random generator for better randomness
};