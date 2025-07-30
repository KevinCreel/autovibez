#include "preset_manager.hpp"
#include "autovibez_app.hpp"
#include "mix_manager.hpp"
#include "string_utils.hpp"
#include <iostream>
#include <random> // Added for random preset selection

PresetManager::PresetManager(projectm_playlist_handle playlist)
    : _playlist(playlist) {
}

void PresetManager::nextPreset() {
    if (_playlist) {
        projectm_playlist_play_next(_playlist, true);
        _currentPresetName = getCurrentPresetName();
        // Preset change notification removed - help overlay shows current preset
    }
}

void PresetManager::previousPreset() {
    if (_playlist) {
        projectm_playlist_play_previous(_playlist, true);
        _currentPresetName = getCurrentPresetName();
        // Preset change notification removed - help overlay shows current preset
    }
}

void PresetManager::randomPreset() {
    if (_playlist) {
        // Use a different approach for random preset since projectm_playlist_play_random doesn't exist
        uint32_t preset_count = projectm_playlist_size(_playlist);
        if (preset_count > 0) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint32_t> dis(0, preset_count - 1);
            uint32_t random_index = dis(gen);
            projectm_playlist_set_position(_playlist, random_index, true);
            _currentPresetName = getCurrentPresetName();
            // Preset change notification removed - help overlay shows current preset
        }
    }
}

void PresetManager::togglePause() {
    _isPaused = !_isPaused;
    // Preset pause/resume notification removed - help overlay shows current state
}

std::string PresetManager::getCurrentPresetName() const {
    // Note: ProjectM-4 doesn't have a direct get_preset function
    // We'll need to track the current preset name manually
    return _currentPresetName;
}

std::string PresetManager::getCurrentPresetPath() const {
    return _currentPresetPath;
}

void PresetManager::setPresetPath(const std::string& path) {
    _currentPresetPath = path;
}

bool PresetManager::isPlaying() const {
    return _isPlaying && !_isPaused;
} 