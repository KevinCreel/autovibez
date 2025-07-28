#include "mix_player.hpp"
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdio>

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
    if (!isValidMP3File(local_path)) {
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
            // Fallback to stream redirection
            null_stream.open("/dev/null");
            if (null_stream.is_open()) {
                original_stderr = std::cerr.rdbuf();
                std::cerr.rdbuf(null_stream.rdbuf());
            }
        }
#endif
    }
    
    // Load and play the music
    current_music = Mix_LoadMUS(local_path.c_str());
    
    // Restore stderr
    if (!_verbose) {
#ifndef _WIN32
        if (original_stderr_file) {
            freopen("/dev/stderr", "w", stderr);
        } else if (original_stderr) {
            std::cerr.rdbuf(original_stderr);
            null_stream.close();
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
    
    // Set volume
    Mix_VolumeMusic((volume * MIX_MAX_VOLUME) / 100);
    
    playing = true;
    current_position = 0;
    
    // Get duration (approximate)
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
        printf("▶️  Resumed playback\n");
        printf("\n");
    } else {
        Mix_PauseMusic();
        printf("⏸️  Paused playback\n");
        printf("\n");
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
    printf("⏹️  Stopped playback\n");
    printf("\n");
    
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
    
    // Only show volume output if not suppressed
    if (!suppress_output) {
        printf("🔊 Volume: %d%%\n", volume);
        printf("\n");
    }
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

bool MixPlayer::isValidMP3File(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Read the first 10 bytes to check for MP3 header
    char header[10];
    file.read(header, 10);
    
    if (file.gcount() < 10) {
        return false;  // File too small
    }
    
    // Check for MP3 sync word (0xFF 0xFB or 0xFF 0xFA or 0xFF 0xF3)
    // MP3 files start with 0xFF followed by a sync byte
    if ((header[0] & 0xFF) == 0xFF && 
        ((header[1] & 0xE0) == 0xE0 || (header[1] & 0xF0) == 0xF0)) {
        return true;
    }
    
    // Check for ID3v2 header (starts with "ID3")
    if (header[0] == 'I' && header[1] == 'D' && header[2] == '3') {
        return true;
    }
    
    // Check for ID3v1 header (at end of file, but we'll check the beginning too)
    // This is less reliable but can help identify some MP3s
    
    return false;
} 