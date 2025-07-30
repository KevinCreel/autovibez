/**
 * projectM -- Milkdrop-esque visualisation SDK
 * Copyright (C)2003-2021 projectM Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE.txt' included within this release
 *
 * projectM-sdl
 * This is an implementation of projectM using libSDL2
 *
 * main.cpp
 * Authors: Created by Mischa Spiegelmock on 6/3/15.
 *
 *
 *	RobertPancoast77@gmail.com :
 * experimental Stereoscopic SBS driver functionality
 * WASAPI loopback implementation
 *
 *
 */

#include "autovibez_app.hpp"
using AutoVibez::Core::AutoVibezApp;
#include "setup.hpp"
#include "mix_metadata.hpp"
#include "mix_manager.hpp"
#include "mix_player.hpp"
#include "mix_downloader.hpp"
#include "path_manager.hpp"
#include <filesystem>

using AutoVibez::Audio::processLoopbackFrame;
using AutoVibez::Audio::cleanupLoopback;
using AutoVibez::Audio::MixPlayer;
using AutoVibez::Data::MixDownloader;
using AutoVibez::Data::MixMetadata;
using AutoVibez::Data::MixManager;
using AutoVibez::Data::Mix;

static int mainLoop(void* userData) {
    std::unique_ptr<AutoVibez::Core::AutoVibezApp> *appRef = static_cast<std::unique_ptr<AutoVibez::Core::AutoVibezApp> *>(userData);
    AutoVibez::Core::AutoVibezApp *app = appRef->get();
    
#if UNLOCK_FPS
    auto start = startUnlockedFPSCounter();
#endif
    
    // frame rate limiter
    int fps = app->fps();
    if (fps <= 0)
        fps = 60;
    const Uint32 frame_delay = 1000/fps;
    Uint32 last_time = SDL_GetTicks();
    
    // Initialize mix manager on startup
    if (!app->isMixManagerInitialized()) {
        app->initMixManager();
    }
    
    // loop
    while (! app->done) {
        // render
        app->renderFrame();
        
        if (app->fakeAudio)
            app->addFakePCM();
        processLoopbackFrame(app);
        
        // Check for auto-play when music ends
        if (app->isMixManagerInitialized()) {
            // Check if current mix has finished
            if (app->getMixManager()->hasFinished()) {
                // Music has finished, trigger auto-play
                app->checkAndAutoPlayNext();
            }
            
            // Also check periodically to ensure music is always playing
            static Uint32 last_check = 0;
            Uint32 current_time = SDL_GetTicks();
            if (current_time - last_check > 5000) { // Check every 5 seconds
                if (!app->getMixManager()->isPlaying() && !app->getMixManager()->isPaused()) {
                    app->checkAndAutoPlayNext();
                }
                last_check = current_time;
            }
        }
        
        // Update crossfade if active
        if (app->isMixManagerInitialized()) {
            app->getMixManager()->updateCrossfade();
        }
        
        // Cleanup completed background downloads
        if (app->isMixManagerInitialized()) {
            app->getMixManager()->cleanupCompletedDownloads();
        }
        
#if UNLOCK_FPS
        advanceUnlockedFPSCounterFrame(start);
#else
        app->pollEvents();
        Uint32 elapsed = SDL_GetTicks() - last_time;
        if (elapsed < frame_delay)
            SDL_Delay(frame_delay - elapsed);
        last_time = SDL_GetTicks();
#endif
    }
    
    return 0;
}

void testMixManager() {
    // Test audio integration
    // Test notification removed - too verbose for normal operation
    
    // Test audio components
    // Test notification removed - too verbose for normal operation
    
    // Test audio player
    MixPlayer player;
    // Test notification removed - too verbose for normal operation
    
    // Test volume control
    player.setVolume(80);
    // Test notification removed - too verbose for normal operation
    
    // Test downloader
    MixDownloader downloader("./test_cache");
    // Test notification removed - too verbose for normal operation
    
    // Test cache management
    // Test notification removed - too verbose for normal operation
    if (std::filesystem::exists("./test_cache")) {
        // Test notification removed - too verbose for normal operation
    } else {
        // Test notification removed - too verbose for normal operation
    }
    
    // Test notification removed - too verbose for normal operation
    
    // Test YAML parsing
    // Test notification removed - too verbose for normal operation
    
    // Test YAML parsing
    MixMetadata metadata;
    std::string test_yaml_content = R"(
mixes:
  - name: Test Mix 1
    url: https://example.com/mix1.mp3
    duration: 300
    artist: Artist 1
    album: Album 1
    genre: Rock
    year: 2020
    cover_art: https://example.com/cover1.jpg
  - name: Test Mix 2
    url: https://example.com/mix2.mp3
    duration: 200
    artist: Artist 2
    album: Album 2
    genre: Pop
    year: 2021
    cover_art: https://example.com/cover2.jpg
)";
    auto mixes = metadata.loadFromYaml(test_yaml_content);
    if (!mixes.empty()) {
        // Test notification removed - too verbose for normal operation
        
        // Test notification removed - too verbose for normal operation
        
        if (!mixes.empty()) {
            const auto& first_mix = mixes[0];
            // Test notification removed - too verbose for normal operation
        }
    } else {
        // Test notification removed - too verbose for normal operation
    }
    
    // Test notification removed - too verbose for normal operation
    
    // Test notification removed - too verbose for normal operation
    // Test notification removed - too verbose for normal operation
    // Test notification removed - too verbose for normal operation
    // Test notification removed - too verbose for normal operation
    // Test notification removed - too verbose for normal operation
    
    // Test notification removed - too verbose for normal operation
    
    // Test notification removed - too verbose for normal operation
}

int main(int argc, char* argv[]) {
    // Check for test mode
    if (argc > 1 && std::string(argv[1]) == "--test-mix") {
        testMixManager();
        return 0;
    }
    
    std::unique_ptr<AutoVibez::Core::AutoVibezApp> app(setupSDLApp());
    
    int status = mainLoop(&app);

    // cleanup
    cleanupLoopback();  // Add this line to clean up WASAPI resources
    SDL_GL_DeleteContext(app->_openGlContext);
#if !FAKE_AUDIO
    if (!app->wasapi) // not currently using WASAPI, so we need to endAudioCapture.
        app->endAudioCapture();
#endif
    
    // Get the window from the app and destroy it properly
    SDL_Window* window = app->getWindow();
    if (window) {
        SDL_DestroyWindow(window);
    }
    
    return status;
}


