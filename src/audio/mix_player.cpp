#include "mix_player.hpp"
#include "mix_metadata.hpp"
#include "path_manager.hpp"
#include "constants.hpp"
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdio>

using AutoVibez::Audio::MixPlayer;

namespace AutoVibez {
namespace Audio {

MixPlayer::MixPlayer()
    : playing(false), current_position(0), duration(0), volume(100), current_music(nullptr) {
    
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
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

bool MixPlayer::playMix(const std::string& local_path, const std::string& title) {
    (void)title; // Parameter not used in current implementation
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
    
    // Temporarily redirect stderr to suppress SDL_mixer warnings
    std::streambuf* original_stderr = nullptr;
    std::ofstream null_stream;
    FILE* original_stderr_file = nullptr;
    
    if (!_verbose) {
#ifndef _WIN32
    // On Unix-like systems, redirect stderr to /dev/null
    original_stderr_file = freopen("/dev/null", "w", stderr);
    if (!original_stderr_file) {
        null_stream.open("/dev/null");
        if (!null_stream.is_open()) {
        }
    }
#endif
    }
    
    current_music = Mix_LoadMUS(local_path.c_str());
    
    if (!_verbose) {
#ifndef _WIN32
        if (freopen("/dev/stderr", "w", stderr) == nullptr) {
        }
#endif
    }
    
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
    
    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / 100);
    
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
        // Playback resume notification removed - help overlay shows current state
    } else {
        Mix_PauseMusic();
        // Playback pause notification removed - help overlay shows current state
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
    // Playback stop notification removed - help overlay shows current state
    
    return true;
}

bool MixPlayer::setVolume(int new_volume) {
    return setVolume(new_volume, false);
}

bool MixPlayer::setVolume(int new_volume, bool suppress_output) {
    if (new_volume < 0) new_volume = 0;
    if (new_volume > 100) new_volume = 100;
    
    volume = new_volume;
    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / 100);
    
    // Volume notification removed - help overlay shows current volume
    return true;
}

int MixPlayer::getCurrentPosition() const {
    if (!playing) return 0;
    
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



} // namespace Audio
} // namespace AutoVibez 