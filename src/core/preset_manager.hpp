#pragma once

#include <projectM-4/playlist.h>
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

}; 