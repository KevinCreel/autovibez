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
    

    
private:
    projectm_playlist_handle _playlist;

}; 