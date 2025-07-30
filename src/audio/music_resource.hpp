#pragma once

#include <SDL2/SDL_mixer.h>
#include <memory>
#include <string>

/**
 * RAII wrapper for SDL_mixer music resources
 * Automatically frees the music resource when the object goes out of scope
 */
class MusicResource {
public:
    struct MusicDeleter {
        void operator()(Mix_Music* music) const {
            if (music) {
                Mix_FreeMusic(music);
            }
        }
    };
    
    using MusicPtr = std::unique_ptr<Mix_Music, MusicDeleter>;
    
    MusicResource() = default;
    
    // Load music from file
    explicit MusicResource(const std::string& file_path) {
        load(file_path);
    }
    
    MusicResource(MusicResource&& other) noexcept = default;
    
    MusicResource& operator=(MusicResource&& other) noexcept = default;
    
    MusicResource(const MusicResource&) = delete;
    MusicResource& operator=(const MusicResource&) = delete;
    
    // Load music from file
    bool load(const std::string& file_path) {
        music_ = MusicPtr(Mix_LoadMUS(file_path.c_str()));
        return music_ != nullptr;
    }
    
    // Get the underlying Mix_Music pointer
    Mix_Music* get() const { return music_.get(); }
    
    bool isLoaded() const { return music_ != nullptr; }
    
    // Get last error
    std::string getLastError() const {
        return music_ ? "" : Mix_GetError();
    }
    
    // Release ownership of the music resource
    Mix_Music* release() {
        return music_.release();
    }
    
private:
    MusicPtr music_;
}; 