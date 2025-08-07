#pragma once

#include <SDL2/SDL_mixer.h>

#include <memory>
#include <string>

#include "audio_utils.hpp"
#include "error_handler.hpp"
#include "mix_metadata.hpp"

namespace AutoVibez {
namespace Audio {

/**
 * @brief Handles audio playback of mix files
 */
class MixPlayer : public ::AutoVibez::Utils::ErrorHandler {
public:
    MixPlayer();
    ~MixPlayer();

    /**
     * @brief Load and play a mix file
     * @param local_path Path to local mix file
     * @return True if successful, false otherwise
     */
    bool playMix(const std::string& local_path);

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

    void setVerbose(bool verbose) {
        _verbose = verbose;
    }

private:
    bool playing;
    int current_position;
    int duration;
    int volume;
    Mix_Music* current_music;
    bool _verbose = false;
};

}  // namespace Audio
}  // namespace AutoVibez