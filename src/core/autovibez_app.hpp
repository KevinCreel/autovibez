/**
 * projectM -- Milkdrop-esque visualisation SDK
 * Copyright (C)2003-2019 projectM Team
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
 * pmSDL.hpp
 * Authors: Created by Mischa Spiegelmock on 2017-09-18.
 *
 */

#pragma once

#include <SDL2/SDL.h>

// Disable LOOPBACK and FAKE audio to enable microphone input
#ifdef _WIN32
#define WASAPI_LOOPBACK 1
#endif /** _WIN32 */
#define FAKE_AUDIO 0

// projectM
#include <projectM-4/playlist.h>
#include <projectM-4/projectM.h>

// projectM SDL
#include "audio_capture.hpp"
#include "loopback.hpp"
#include "opengl.h"
#include "setup.hpp"

// Mix management
#include "config_manager.hpp"
#include "help_overlay.hpp"
#include "message_overlay_wrapper.hpp"
#include "mix_downloader.hpp"
#include "mix_manager.hpp"
#include "mix_metadata.hpp"

// New modular components
#include "constants.hpp"
#include "path_manager.hpp"
#include "preset_manager.hpp"

#if defined _MSC_VER
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <atomic>
#include <fstream>
#include <future>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#ifdef _WIN32
#ifdef WASAPI_LOOPBACK
#include <audioclient.h>
#include <avrt.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <mmsystem.h>
#include <stdio.h>
#include <windows.h>

#define LOG(format, ...) wprintf(format L"\n", __VA_ARGS__)
#define ERR(format, ...) LOG(L"Error: " format, __VA_ARGS__)
#endif /** WASAPI_LOOPBACK */
#endif /** _WIN32 */

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif /** _WIN32 */

// DATADIR_PATH should be set by the root Makefile if this is being
// built with autotools.

namespace AutoVibez {
namespace Core {

class AutoVibezApp {
public:
    AutoVibezApp(SDL_GLContext glCtx, const std::string& presetPath, const std::string& texturePath,
                 int audioDeviceIndex = 0, bool showFps = false);

    ~AutoVibezApp();

    void initialize(SDL_Window* window);

    int toggleAudioInput();
    int initializeAudioInput();
    void beginAudioCapture();
    void endAudioCapture();
    void stretchMonitors();
    void nextMonitor();
    void toggleFullScreen();
    void syncFullscreenState();  // Synchronize _isFullScreen with actual SDL state
    void resizeWindow(unsigned int width, unsigned int height);

    void renderFrame();
    void pollEvents();
    bool keymod = false;
    std::string getActivePresetName();
    projectm_handle projectM();
    float getBeatSensitivity() const;

    // Help and UI
    void cycleAudioDevice();

    // Mix management

    void initMixManager();
    void handleMixControls(SDL_Event* event);
    void autoPlayFromLocalDatabase();
    void startBackgroundDownloads();
    void checkAndAutoPlayNext();
    bool isMixManagerInitialized() const {
        return _mixManagerInitialized;
    }
    AutoVibez::Data::MixManager* getMixManager() {
        return _mixManager.get();
    }

    // Window management
    SDL_Window* getWindow() const {
        return _sdlWindow;
    }

    // Help Overlay
    void initHelpOverlay();
    void renderHelpOverlay();
    void updateHelpOverlayInfo();

    // Message Overlay
    void initMessageOverlay();
    void renderMessageOverlay();

    // New modular component accessors
    PresetManager* getPresetManager() {
        return _presetManager.get();
    }

    // Beat sensitivity
    void setBeatSensitivity(float sensitivity);

    bool done{false};
    bool mouseDown{false};
    bool wasapi{false};  // Used to track if wasapi is currently active. This bool will allow us to run a WASAPI app and
                         // still toggle to microphone inputs.
    bool fakeAudio{false};  // Used to track fake audio, so we can turn it off and on.
    bool stretch{false};    // used for toggling stretch mode

    SDL_GLContext _openGlContext{nullptr};
    SDL_Window* _sdlWindow{nullptr};

public:
    static void presetSwitchedEvent(bool isHardCut, uint32_t index, void* context);
    static void audioInputCallbackF32(void* userdata, unsigned char* stream, int len);

    // Audio-related getters
    unsigned short getAudioChannelsCount() const {
        return _audioChannelsCount;
    }
    projectm_handle getProjectM() const {
        return _projectM;
    }

    // Friend function for audio callback
    friend void AutoVibez::Audio::audioInputCallbackF32(void* userData, const float* buffer, int len);

private:
    void UpdateWindowTitle();

    // Mouse wheel function declarations removed
    void keyHandler(SDL_Event*);

    projectm_handle _projectM{nullptr};
    projectm_playlist_handle _playlist{nullptr};

    bool _isFullScreen{false};
    size_t _width{0};
    size_t _height{0};

    unsigned short _audioChannelsCount{0};
    unsigned int _numAudioDevices{0};
    SDL_AudioDeviceID _audioDeviceId{0};

    std::string _presetName;  //!< Current preset name

    int _selectedAudioDeviceIndex{0};  //!< Selected audio device index

    // Mix management
    std::unique_ptr<AutoVibez::Data::MixManager> _mixManager;

    AutoVibez::Data::Mix _currentMix;
    bool _mixManagerInitialized;
    bool _hadMixesOnStartup;
    bool _volumeKeyPressed{false};               // Track if volume key is being held
    bool _manualPresetChange{false};             // Track if preset change was manual
    int _previousVolume{Constants::MAX_VOLUME};  // Store volume before mute

    // Help Overlay
    std::unique_ptr<AutoVibez::UI::HelpOverlay> _helpOverlay;

    // Message Overlay
    std::unique_ptr<AutoVibez::UI::MessageOverlayWrapper> _messageOverlay;

    // New modular components
    std::unique_ptr<PresetManager> _presetManager;

    // Thread safety members
    std::future<void> _backgroundTask;
    std::atomic<bool> _backgroundTaskRunning{false};

    void handleWindowEvent(const SDL_Event& evt);
    // Mouse wheel event handler removed
    void handleKeyDownEvent(const SDL_Event& evt);
    void handleKeyUpEvent(const SDL_Event& evt);
    void handleMouseButtonDownEvent(const SDL_Event& evt);
    void handleMouseButtonUpEvent(const SDL_Event& evt);
    void handleQuitEvent();
};

}  // namespace Core
}  // namespace AutoVibez
