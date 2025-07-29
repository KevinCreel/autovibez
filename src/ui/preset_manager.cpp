#include "preset_manager.hpp"
#include "string_utils.hpp"
#include <iostream>

PresetManager::PresetManager(projectm_playlist_handle playlist)
    : _playlist(playlist) {
}

void PresetManager::nextPreset() {
    projectm_playlist_play_next(_playlist, true);
    _currentPresetName = getCurrentPresetName();
    printf("⏭️  Next preset: %s\n", StringUtils::formatPresetName(_currentPresetName).c_str());
}

void PresetManager::previousPreset() {
    projectm_playlist_play_previous(_playlist, true);
    _currentPresetName = getCurrentPresetName();
    printf("⏮️  Previous preset: %s\n", StringUtils::formatPresetName(_currentPresetName).c_str());
}

void PresetManager::randomPreset() {
    // For now, just go to next preset
    // TODO: Implement true random selection
    nextPreset();
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

void PresetManager::togglePause() {
    _isPaused = !_isPaused;
    if (_isPaused) {
        printf("⏸️  Preset paused\n");
    } else {
        printf("▶️  Preset resumed\n");
    }
} 