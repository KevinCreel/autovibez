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
#include "constants.hpp"
using AutoVibez::Core::AutoVibezApp;

#include "mix_downloader.hpp"
#include "mix_manager.hpp"
#include "mix_metadata.hpp"
#include "mix_player.hpp"
#include "path_manager.hpp"
#include "setup.hpp"

using AutoVibez::Audio::cleanupLoopback;
using AutoVibez::Audio::MixPlayer;
using AutoVibez::Audio::processLoopbackFrame;
using AutoVibez::Data::Mix;
using AutoVibez::Data::MixDownloader;
using AutoVibez::Data::MixManager;
using AutoVibez::Data::MixMetadata;

// Helper function to execute code only if mix manager is initialized
template <typename Func>
static void executeIfMixManagerInitialized(AutoVibez::Core::AutoVibezApp* app, Func&& func) {
    if (app->isMixManagerInitialized()) {
        func();
    }
}

static int mainLoop(void* userData) {
    std::unique_ptr<AutoVibez::Core::AutoVibezApp>* appRef =
        static_cast<std::unique_ptr<AutoVibez::Core::AutoVibezApp>*>(userData);
    AutoVibez::Core::AutoVibezApp* app = appRef->get();

    // frame rate limiter
    const Uint32 frame_delay = Constants::FRAME_DELAY_MS;
    Uint32 last_time = SDL_GetTicks();

    // Initialize mix manager on startup
    if (!app->isMixManagerInitialized()) {
        app->initMixManager();
    }

    // loop
    while (!app->done) {
        // render
        app->renderFrame();

        processLoopbackFrame(app);

        executeIfMixManagerInitialized(app, [&]() {
            if (app->getMixManager()->hasFinished()) {
                app->checkAndAutoPlayNext();
            }

            static Uint32 last_check = 0;
            Uint32 current_time = SDL_GetTicks();
            if (current_time - last_check > Constants::DEFAULT_CHECK_INTERVAL_MS) {
                if (!app->getMixManager()->isPlaying() && !app->getMixManager()->isPaused()) {
                    app->checkAndAutoPlayNext();
                }

                last_check = current_time;
            }
        });

        executeIfMixManagerInitialized(app, [&]() { app->getMixManager()->updateCrossfade(); });

        executeIfMixManagerInitialized(app, [&]() { app->getMixManager()->cleanupCompletedDownloads(); });

        app->pollEvents();
        Uint32 elapsed = SDL_GetTicks() - last_time;
        if (elapsed < frame_delay) {
            SDL_Delay(frame_delay - elapsed);
        }
        last_time = SDL_GetTicks();
    }

    return 0;
}

int main(int argc, char* argv[]) {
    std::unique_ptr<AutoVibez::Core::AutoVibezApp> app(setupSDLApp());

    int status = mainLoop(&app);

    // cleanup
    cleanupLoopback();  // Add this line to clean up WASAPI resources
    SDL_GL_DeleteContext(app->_openGlContext);
#if !FAKE_AUDIO
    if (!app->wasapi) {  // not currently using WASAPI, so we need to endAudioCapture.
        app->endAudioCapture();
    }
#endif

    // Get the window from the app and destroy it properly
    SDL_Window* window = app->getWindow();
    if (window) {
        SDL_DestroyWindow(window);
    }

    return status;
}