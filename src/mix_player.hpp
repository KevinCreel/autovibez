#pragma once

#include "mix_metadata.hpp"
#include <SDL2/SDL_mixer.h>
#include <string>

/**
 * @brief Handles audio playback of mix files
 */
class MixPlayer {
public:
    MixPlayer();
    ~MixPlayer();
    
    /**
     * @brief Load and play a mix file
     * @param local_path Path to local mix file
     * @param title Optional title to display (if empty, shows filename)
     * @return True if successful, false otherwise
     */
    bool playMix(const std::string& local_path, const std::string& title = "");
    
    /**
     * @brief Pause/resume playback
     * @return True if successful, false otherwise
     */
    bool togglePause();
    
    /**
     * @brief Stop playback
     * @return True if successful, false otherwise
     */
    bool stop();
    
    /**
     * @brief Set volume
     * @param volume Volume (0-100)
     * @return True if successful, false otherwise
     */
    bool setVolume(int new_volume);
    
    /**
     * @brief Set volume with optional output suppression
     * @param volume Volume (0-100)
     * @param suppress_output Whether to suppress volume output messages
     * @return True if successful, false otherwise
     */
    bool setVolume(int new_volume, bool suppress_output);
    
    /**
     * @brief Get current volume
     * @return Current volume (0-100)
     */
    int getVolume() const;
    
    /**
     * @brief Get current position
     * @return Position in seconds
     */
    int getCurrentPosition() const;
    
    /**
     * @brief Get total duration
     * @return Duration in seconds
     */
    int getDuration() const;
    
    /**
     * @brief Check if playing
     * @return True if playing, false otherwise
     */
    bool isPlaying() const;
    
    /**
     * @brief Check if paused
     * @return True if paused, false otherwise
     */
    bool isPaused() const;
    
    /**
     * @brief Check if music has finished and update internal state
     * @return True if music has finished, false otherwise
     */
    bool hasFinished();
    
    /**
     * @brief Get the last error message
     * @return Error message string
     */
    std::string getLastError() const;

    /**
     * @brief Check if a file is a valid MP3
     * @param file_path Path to the file to check
     * @return True if valid MP3, false otherwise
     */
    bool isValidMP3File(const std::string& file_path);

    void setVerbose(bool verbose) { _verbose = verbose; }

private:
    std::string last_error;
    bool playing;
    int current_position;
    int duration;
    int volume;
    Mix_Music* current_music; // Store current music object for seeking
    bool _verbose = false;
}; 