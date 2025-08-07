#include "mix_player.hpp"

#include <SDL2/SDL_mixer.h>

#include <filesystem>
#include <fstream>

#include "constants.hpp"
#include "mix_metadata.hpp"
#include "path_manager.hpp"

using AutoVibez::Audio::MixPlayer;

namespace AutoVibez {
namespace Audio {

MixPlayer::MixPlayer()
    : playing(false), current_position(0), duration(0), volume(Constants::MAX_VOLUME), current_music(nullptr) {
    // Initialize SDL_mixer
    if (Mix_OpenAudio(Constants::DEFAULT_SAMPLE_RATE, MIX_DEFAULT_FORMAT, Constants::DEFAULT_CHANNELS,
                      Constants::DEFAULT_BUFFER_SIZE) < 0) {
        last_error = "Failed to initialize SDL_mixer: " + std::string(Mix_GetError());
        return;
    }

    // Set volume
    Mix_Volume(-1, MIX_MAX_VOLUME);
}

MixPlayer::~MixPlayer() {
    if (playing) {
        Mix_HaltMusic();
    }
    if (current_music) {
        Mix_FreeMusic(current_music);
        current_music = nullptr;
    }
    Mix_CloseAudio();
}

bool MixPlayer::playMix(const std::string& local_path) {
    last_error.clear();

    if (!std::filesystem::exists(local_path)) {
        last_error = "File does not exist: " + local_path;
        return false;
    }

    // Stop any currently playing music
    if (playing) {
        Mix_HaltMusic();
        if (current_music) {
            Mix_FreeMusic(current_music);
            current_music = nullptr;
        }
    }

    // Validate that the file is actually an MP3
    if (!AutoVibez::Utils::AudioUtils::isValidMP3File(local_path)) {
        last_error = "File is not a valid MP3: " + local_path;
        return false;
    }

    current_music = Mix_LoadMUS(local_path.c_str());

    if (!current_music) {
        last_error = "Failed to load music: " + std::string(Mix_GetError());
        return false;
    }

    if (Mix_PlayMusic(current_music, 0) == -1) {
        last_error = "Failed to play music: " + std::string(Mix_GetError());
        Mix_FreeMusic(current_music);
        current_music = nullptr;
        return false;
    }

    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / Constants::MAX_VOLUME);

    playing = true;
    current_position = 0;

    duration = Mix_MusicDuration(current_music);

    return true;
}

bool MixPlayer::togglePause() {
    if (!playing) {
        last_error = "No music is currently playing";
        return false;
    }

    if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
    } else {
        Mix_PauseMusic();
    }

    return true;
}

bool MixPlayer::stop() {
    if (!playing) {
        return true;
    }

    Mix_HaltMusic();
    if (current_music) {
        Mix_FreeMusic(current_music);
        current_music = nullptr;
    }
    playing = false;
    current_position = 0;

    return true;
}

bool MixPlayer::setVolume(int new_volume) {
    return setVolume(new_volume, false);
}

bool MixPlayer::setVolume(int new_volume, bool suppress_output) {
    if (new_volume < Constants::MIN_VOLUME)
        new_volume = Constants::MIN_VOLUME;
    if (new_volume > Constants::MAX_VOLUME)
        new_volume = Constants::MAX_VOLUME;

    volume = new_volume;
    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / Constants::MAX_VOLUME);

    return true;
}

int MixPlayer::getCurrentPosition() const {
    if (!playing)
        return 0;

    // Get current position from SDL_mixer
    return Mix_GetMusicPosition(nullptr);
}

bool MixPlayer::isPlaying() const {
    return playing && !Mix_PausedMusic() && Mix_PlayingMusic();
}

bool MixPlayer::hasFinished() {
    if (playing && !Mix_PlayingMusic() && !Mix_PausedMusic()) {
        playing = false;
        if (current_music) {
            Mix_FreeMusic(current_music);
            current_music = nullptr;
        }
        return true;
    }
    return false;
}

bool MixPlayer::isPaused() const {
    return playing && Mix_PausedMusic();
}

int MixPlayer::getVolume() const {
    return volume;
}

int MixPlayer::getDuration() const {
    return duration;
}

std::string MixPlayer::getLastError() const {
    return last_error;
}

}  // namespace Audio
}  // namespace AutoVibez