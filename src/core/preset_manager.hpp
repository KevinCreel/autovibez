#pragma once

#include <projectM-4/playlist.h>
#include <string>

class PresetManager {
public:
    explicit PresetManager(projectm_playlist_handle playlist);
    
    /**
     * Navigate to next preset
     */
    void nextPreset();
    
    /**
     * Navigate to previous preset
     */
    void previousPreset();
    
    /**
     * Jump to random preset
     */
    void randomPreset();
    
    /**
     * Get current preset name
     */
    std::string getCurrentPresetName() const;
    
    /**
     * Get current preset path
     */
    std::string getCurrentPresetPath() const;
    
    /**
     * Set preset path
     */
    void setPresetPath(const std::string& path);
    
    /**
     * Check if preset is currently playing
     */
    bool isPlaying() const;
    
    /**
     * Pause/resume preset playback
     */
    void togglePause();
    
    /**
     * Get playlist handle
     */
    projectm_playlist_handle getPlaylist() const { return _playlist; }
    
private:
    projectm_playlist_handle _playlist;
    std::string _currentPresetName;
    std::string _currentPresetPath;
    bool _isPlaying{true};
    bool _isPaused{false};
}; 