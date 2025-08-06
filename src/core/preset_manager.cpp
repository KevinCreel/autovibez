#include "preset_manager.hpp"
#include "autovibez_app.hpp"
#include "mix_manager.hpp"
#include <iostream>
#include <random> // Added for random preset selection

PresetManager::PresetManager(projectm_playlist_handle playlist)
    : _playlist(playlist)
    , _randomGenerator(std::random_device{}()) {
}

void PresetManager::randomPreset() {
    if (_playlist) {
        // Use a different approach for random preset since projectm_playlist_play_random doesn't exist
        uint32_t preset_count = projectm_playlist_size(_playlist);
        if (preset_count > 0) {
            std::uniform_int_distribution<uint32_t> dis(0, preset_count - 1);
            uint32_t random_index = dis(_randomGenerator);
            projectm_playlist_set_position(_playlist, random_index, true);
        }
    }
} 