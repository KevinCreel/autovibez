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
 * pmSDL.cpp
 * Authors: Created by Mischa Spiegelmock on 2017-09-18.
 *
 *
 * experimental Stereoscopic SBS driver functionality by
 *	RobertPancoast77@gmail.com
 */

#include "autovibez_app.hpp"

#include <filesystem>
#include <thread>
#include <vector>

#include "config_manager.hpp"
#include "constants.hpp"
#include "mix_downloader.hpp"
#include "mix_manager.hpp"
#include "mix_metadata.hpp"
#include "path_manager.hpp"
#include "setup.hpp"
#if defined _MSC_VER
#include <direct.h>
#else
#include <sys/stat.h>
#endif

using AutoVibez::Data::MixManager;

using AutoVibez::Data::ConfigFile;
using AutoVibez::Data::Mix;
using AutoVibez::UI::HelpOverlay;

namespace AutoVibez {
namespace Core {

AutoVibezApp::AutoVibezApp(SDL_GLContext glCtx, const std::string& presetPath, const std::string& texturePath,
                           int audioDeviceIndex, bool showFps)
    : _openGlContext(glCtx),
      _projectM(projectm_create()),
      _playlist(projectm_playlist_create(_projectM)),
      _selectedAudioDeviceIndex(audioDeviceIndex),
      _mixManagerInitialized(false),
      _hadMixesOnStartup(false) {
    projectm_get_window_size(_projectM, &_width, &_height);
    projectm_playlist_set_preset_switched_event_callback(_playlist, &AutoVibezApp::presetSwitchedEvent,
                                                         static_cast<void*>(this));
    projectm_playlist_add_path(_playlist, presetPath.c_str(), true, false);

    // Initialize PresetManager
    _presetManager = std::make_unique<PresetManager>(_playlist);

    // Load a random preset on startup
    if (_presetManager) {
        _presetManager->randomPreset();
    }
}

AutoVibezApp::~AutoVibezApp() {
    // Stop any playing music and clean up MixManager
    if (_mixManager) {
        _mixManager->stop();
    }

    if (_backgroundTaskRunning.load()) {
        if (_backgroundTask.valid()) {
            _backgroundTask.wait();
        }
        _backgroundTaskRunning.store(false);
    }

    projectm_playlist_destroy(_playlist);
    _playlist = nullptr;
    projectm_destroy(_projectM);
    _projectM = nullptr;
}

/* Stretch projectM across multiple monitors */
void AutoVibezApp::stretchMonitors() {
    int displayCount = SDL_GetNumVideoDisplays();
    if (displayCount >= 2) {
        std::vector<SDL_Rect> displayBounds;
        for (int i = 0; i < displayCount; i++) {
            displayBounds.push_back(SDL_Rect());
            SDL_GetDisplayBounds(i, &displayBounds.back());
        }

        int mostXLeft = 0;
        int mostXRight = 0;
        int mostYUp = 0;
        int mostYDown = 0;

        for (int i = 0; i < displayCount; i++) {
            if (displayBounds[i].x < mostXLeft) {
                mostXLeft = displayBounds[i].x;
            }
            if ((displayBounds[i].x + displayBounds[i].w) > mostXRight) {
                mostXRight = displayBounds[i].x + displayBounds[i].w;
            }
        }
        for (int i = 0; i < displayCount; i++) {
            if (displayBounds[i].y < mostYUp) {
                mostYUp = displayBounds[i].y;
            }
            if ((displayBounds[i].y + displayBounds[i].h) > mostYDown) {
                mostYDown = displayBounds[i].y + displayBounds[i].h;
            }
        }

        int mostWide = abs(mostXLeft) + abs(mostXRight);
        int mostHigh = abs(mostYUp) + abs(mostYDown);

        SDL_SetWindowPosition(_sdlWindow, mostXLeft, mostYUp);
        SDL_SetWindowSize(_sdlWindow, mostWide, mostHigh);
    }
}

/* Moves projectM to the next monitor */
void AutoVibezApp::nextMonitor() {
    int displayCount = SDL_GetNumVideoDisplays();
    int currentWindowIndex = SDL_GetWindowDisplayIndex(_sdlWindow);
    if (displayCount >= 2) {
        std::vector<SDL_Rect> displayBounds;
        int nextWindow = currentWindowIndex + 1;
        if (nextWindow >= displayCount) {
            nextWindow = 0;
        }

        for (int i = 0; i < displayCount; i++) {
            displayBounds.push_back(SDL_Rect());
            SDL_GetDisplayBounds(i, &displayBounds.back());
        }
        SDL_SetWindowPosition(_sdlWindow, displayBounds[nextWindow].x, displayBounds[nextWindow].y);
        SDL_SetWindowSize(_sdlWindow, displayBounds[nextWindow].w, displayBounds[nextWindow].h);
    }
}

void AutoVibezApp::syncFullscreenState() {
    Uint32 flags = SDL_GetWindowFlags(_sdlWindow);
    _isFullScreen = (flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void AutoVibezApp::toggleFullScreen() {
    if (_isFullScreen) {
        SDL_SetWindowFullscreen(_sdlWindow, 0);
        _isFullScreen = false;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        if (_helpOverlay) {
            _helpOverlay->setFullscreenState(false);
        }
    } else {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_SetWindowFullscreen(_sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        _isFullScreen = true;
        if (_helpOverlay) {
            _helpOverlay->setFullscreenState(true);
        }
    }
}

void AutoVibezApp::keyHandler(SDL_Event* sdl_evt) {
    if (!_mixManagerInitialized) {
        initMixManager();
    }

    // Handle mix controls first
    handleMixControls(sdl_evt);

    SDL_Keymod sdl_mod = (SDL_Keymod)sdl_evt->key.keysym.mod;
    SDL_Keycode sdl_keycode = sdl_evt->key.keysym.sym;

    // Left or Right Gui or Left Ctrl
    if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL) {
        keymod = true;
    }

    switch (sdl_keycode) {
        case SDLK_q:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL) {
                // cmd/ctrl-q = quit
                done = true;
                return;
            }
            break;

        case SDLK_i:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL) {
                toggleAudioInput();
                return;  // handled
            }
            break;

        case SDLK_s:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL) {
                // command-s: [s]tretch monitors
                if (!this->stretch) {
                    stretchMonitors();
                    this->stretch = true;
                } else {
                    toggleFullScreen();  // else, just toggle full screen so we leave stretch mode.
                    this->stretch = false;
                }
                return;  // handled
            }

        case SDLK_m:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL) {
                // command-m: change [m]onitor
                nextMonitor();
                this->stretch = false;
                return;  // handled
            }

        case SDLK_f:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL) {
                // command-f: fullscreen
                toggleFullScreen();
                this->stretch = false;
                return;  // handled
            }
            break;

        case SDLK_UP:
            if (_mixManagerInitialized && _mixManager->isPlaying()) {
                // Volume up handled in handleMixControls
            }
            break;

        case SDLK_DOWN:
            if (_mixManagerInitialized && _mixManager->isPlaying()) {
                // Volume down handled in handleMixControls
            }
            break;

        case SDLK_EQUALS:
        case SDLK_PLUS: {
            float newSensitivity = getBeatSensitivity() + 0.1f;
            if (newSensitivity > 1.0f)
                newSensitivity = 1.0f;
            setBeatSensitivity(newSensitivity);
        } break;

        case SDLK_MINUS: {
            float newSensitivity = getBeatSensitivity() - 0.1f;
            if (newSensitivity < 0.0f)
                newSensitivity = 0.0f;
            setBeatSensitivity(newSensitivity);
        } break;

        case SDLK_F11:
            // F11: toggle fullscreen
            toggleFullScreen();
            break;

        case SDLK_h:
            // H: toggle help overlay
            if (_helpOverlay) {
                _helpOverlay->toggle();
            }
            break;

        case SDLK_TAB:
            // Tab: cycle through audio devices
            cycleAudioDevice();
            break;

        case SDLK_SPACE:
            // SPACE: Pause/Resume (universal media standard)
            if (_mixManagerInitialized) {
                _mixManager->togglePause();
            }
            break;

        case SDLK_LEFTBRACKET:
            // [: Previous preset
            _manualPresetChange = true;
            projectm_playlist_play_previous(_playlist, true);
            break;

        case SDLK_RIGHTBRACKET:
            // ]: Next preset
            _manualPresetChange = true;
            projectm_playlist_play_next(_playlist, true);
            {
                std::string preset_name = getActivePresetName();
                size_t last_slash = preset_name.find_last_of('/');
                if (last_slash != std::string::npos) {
                    preset_name = preset_name.substr(last_slash + 1);
                }
            }
            break;

        case SDLK_g:
            // G: Random mix in current genre
            if (_mixManagerInitialized) {
                // Check for modifier keys
                if (sdl_mod & KMOD_LSHIFT || sdl_mod & KMOD_RSHIFT) {
                    // Shift+G: Switch to random genre
                    std::string newGenre = _mixManager->getRandomGenre();

                    // Play a random mix in the new genre
                    Mix genreMix = _mixManager->getRandomMixByGenre(newGenre, _currentMix.id);
                    if (!genreMix.id.empty()) {
                        _mixManager->downloadAndPlayMix(genreMix);
                        _currentMix = genreMix;
                    }
                } else {
                    // G: Random mix in current mix's genre
                    if (!_currentMix.id.empty() && !_currentMix.genre.empty()) {
                        Mix genreMix = _mixManager->getRandomMixByGenre(_currentMix.genre, _currentMix.id);
                        if (!genreMix.id.empty()) {
                            _mixManager->downloadAndPlayMix(genreMix);
                            _currentMix = genreMix;
                        }
                    }
                }
            }
            break;
    }
}

void AutoVibezApp::resizeWindow(unsigned int width_, unsigned int height_) {
    _width = width_;
    _height = height_;

    // Hide cursor if window size equals desktop size
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        SDL_ShowCursor(_isFullScreen ? SDL_DISABLE : SDL_ENABLE);
    }

    projectm_set_window_size(_projectM, _width, _height);
}

// --- Refactored pollEvents ---
void AutoVibezApp::pollEvents() {
    SDL_Event evt;
    while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
            case SDL_WINDOWEVENT:
                handleWindowEvent(evt);
                break;
            case SDL_KEYDOWN:
                handleKeyDownEvent(evt);
                break;
            case SDL_KEYUP:
                handleKeyUpEvent(evt);
                break;
            case SDL_MOUSEBUTTONDOWN:
                handleMouseButtonDownEvent(evt);
                break;
            case SDL_MOUSEBUTTONUP:
                handleMouseButtonUpEvent(evt);
                break;
            case SDL_QUIT:
                handleQuitEvent(evt);
                break;
            default:
                break;
        }
    }
}

// --- Private event handlers ---
void AutoVibezApp::handleWindowEvent(const SDL_Event& evt) {
    int w, h;
    SDL_GL_GetDrawableSize(_sdlWindow, &w, &h);
    switch (evt.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            resizeWindow(w, h);
            break;
        default:
            break;
    }
}

void AutoVibezApp::handleKeyDownEvent(const SDL_Event& evt) {
    keyHandler(const_cast<SDL_Event*>(&evt));
}

void AutoVibezApp::handleKeyUpEvent(const SDL_Event& evt) {
    if (_volumeKeyPressed && (evt.key.keysym.sym == SDLK_UP || evt.key.keysym.sym == SDLK_DOWN)) {
        _volumeKeyPressed = false;
    }
}

void AutoVibezApp::handleMouseButtonDownEvent(const SDL_Event& evt) {
    if (evt.button.button == SDL_BUTTON_LEFT) {
        if (!mouseDown) {
            mouseDown = true;
        }
    } else if (evt.button.button == SDL_BUTTON_RIGHT) {
        mouseDown = false;
        if (keymod) {
            keymod = false;
            return;
        }
    }
}

void AutoVibezApp::handleMouseButtonUpEvent(const SDL_Event& evt) {
    mouseDown = false;
}

void AutoVibezApp::handleQuitEvent(const SDL_Event&) {
    done = true;
}

void AutoVibezApp::renderFrame() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_opengl_render_frame(_projectM);

    // Render overlays
    renderHelpOverlay();

    SDL_GL_SwapWindow(_sdlWindow);
}

void AutoVibezApp::initialize(SDL_Window* window) {
    _sdlWindow = window;
    projectm_set_window_size(_projectM, _width, _height);

#ifdef WASAPI_LOOPBACK
    wasapi = true;
#endif

    // Initialize help overlay
    initHelpOverlay();
}

void AutoVibezApp::initHelpOverlay() {
    if (!_helpOverlay) {
        _helpOverlay = std::make_unique<HelpOverlay>();
        if (_helpOverlay) {
            _helpOverlay->init(_sdlWindow, _openGlContext);
        }
    }
}

void AutoVibezApp::renderHelpOverlay() {
    if (_helpOverlay) {
        // Update help overlay with current information
        updateHelpOverlayInfo();
        _helpOverlay->render();
    }
}

void AutoVibezApp::updateHelpOverlayInfo() {
    if (!_helpOverlay)
        return;

    // Update current preset
    std::string currentPreset = getActivePresetName();
    if (!currentPreset.empty()) {
        // Extract just the filename for display
        size_t last_slash = currentPreset.find_last_of('/');
        if (last_slash != std::string::npos) {
            currentPreset = currentPreset.substr(last_slash + 1);
        }
        _helpOverlay->setCurrentPreset(currentPreset);
    }

    // Update current mix info
    if (_mixManager && !_currentMix.id.empty()) {
        _helpOverlay->setCurrentMix(_currentMix.artist, _currentMix.title, _currentMix.genre);
    }

    // Update volume level
    if (_mixManager) {
        _helpOverlay->setVolumeLevel(_mixManager->getVolume());
    }

    // Update audio device
    const char* deviceName = nullptr;
    if (_selectedAudioDeviceIndex >= 0 && _selectedAudioDeviceIndex < _numAudioDevices) {
        deviceName = SDL_GetAudioDeviceName(_selectedAudioDeviceIndex, SDL_TRUE);
    }
    if (deviceName) {
        _helpOverlay->setAudioDevice(deviceName);
    } else {
        // Show default device indicator
        _helpOverlay->setAudioDevice(StringConstants::DEFAULT_AUDIO_DEVICE);
    }

    // Update beat sensitivity
    _helpOverlay->setBeatSensitivity(getBeatSensitivity());

    // Update mix table data
    if (_mixManager) {
        auto mixes = _mixManager->getAllMixes();
        _helpOverlay->setMixTableData(mixes);
    }
}

std::string AutoVibezApp::getActivePresetName() {
    unsigned int index = projectm_playlist_get_position(_playlist);
    if (index) {
        auto presetName = projectm_playlist_item(_playlist, index);
        std::string presetNameString(presetName);
        projectm_playlist_free_string(presetName);
        return presetNameString;
    }
    return {};
}

void AutoVibezApp::presetSwitchedEvent(bool isHardCut, unsigned int index, void* context) {
    AutoVibezApp* app = static_cast<AutoVibezApp*>(context);
    if (!app) {
        return;
    }

    // Get the current preset name
    const char* presetName = projectm_playlist_item(app->_playlist, index);
    if (presetName) {
        std::string presetNameString(presetName);
        app->_presetName = presetNameString;
        projectm_playlist_free_string(const_cast<char*>(presetName));
    }
}

projectm_handle AutoVibezApp::projectM() {
    return _projectM;
}

float AutoVibezApp::getBeatSensitivity() const {
    return projectm_get_beat_sensitivity(_projectM);
}

void AutoVibezApp::setBeatSensitivity(float sensitivity) {
    projectm_set_beat_sensitivity(_projectM, sensitivity);
}

void AutoVibezApp::UpdateWindowTitle() {
    std::string title = StringConstants::APP_TITLE;
    if (projectm_get_preset_locked(_projectM)) {
        title.append(StringConstants::LOCKED_INDICATOR);
    }

    SDL_SetWindowTitle(_sdlWindow, title.c_str());
}

void AutoVibezApp::cycleAudioDevice() {
    int numDevices = SDL_GetNumAudioDevices(SDL_TRUE);

    // Handle edge cases - no devices available
    if (numDevices <= 0) {
        SDL_Log("No audio capture devices available");
        return;
    }

    // Calculate next device index with proper bounds checking
    int nextAudioDeviceId = ((_selectedAudioDeviceIndex + 2) % (numDevices + 1)) - 1;

    // Validate the new index
    if (nextAudioDeviceId < -1 || nextAudioDeviceId >= numDevices) {
        // Fallback to default device (-1)
        nextAudioDeviceId = -1;
    }

    // Start recording with new device
    _selectedAudioDeviceIndex = nextAudioDeviceId;
    if (initializeAudioInput()) {
        beginAudioCapture();
    }
}

// Mix management methods
void AutoVibezApp::initMixManager() {
    if (_mixManagerInitialized)
        return;

    std::string db_path = PathManager::getStateDirectory() + "/autovibez_mixes.db";
    std::string mixes_dir = PathManager::getMixesDirectory();

    _mixManager = std::make_unique<MixManager>(db_path, mixes_dir);

    // Set up callback for when first mix is added to empty database
    _mixManager->setFirstMixAddedCallback([this](const AutoVibez::Data::Mix& mix) {
        // Only auto-play if we started with an empty database
        if (!_hadMixesOnStartup) {
            if (_mixManager->playMix(mix)) {
                _currentMix = mix;
            }
        }
    });

    // Initialize database first (fast operation)
    if (!_mixManager->initialize()) {
        return;
    }

    // Load configuration (fast operation)
    std::string configFilePath = findConfigFile();
    std::string yaml_url;
    std::string preferred_genre;

    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);

        // Load preferred genre from config
        config.readInto(preferred_genre, "preferred_genre");
        _mixManager->setCurrentGenre(preferred_genre);

        // Show current audio device
        int audioDeviceIndex = config.getAudioDeviceIndex();
        const char* deviceName = SDL_GetAudioDeviceName(audioDeviceIndex, SDL_TRUE);
        if (deviceName) {
        } else {
        }

        // Get YAML URL
        yaml_url = config.getMixesUrl();
    }

    // Mark as initialized early so UI can work
    _mixManagerInitialized = true;

    // Check if there were mixes in the database when the app started
    _hadMixesOnStartup = !_mixManager->getAllMixes().empty();

    // IMMEDIATELY try to play from local database (non-blocking)
    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);
        if (config.getAutoDownload()) {
            // Try to play existing mix immediately
            autoPlayFromLocalDatabase();
        }
    }

    // Load mix metadata in background (non-blocking)
    if (!yaml_url.empty() && !_backgroundTaskRunning.load()) {
        _backgroundTaskRunning.store(true);
        _backgroundTask = std::async(std::launch::async, [this, yaml_url, configFilePath]() {
            // Load mix metadata from YAML
            if (_mixManager->loadMixMetadata(yaml_url)) {
                // Check for new mixes from remote YAML in background (non-blocking)
                _mixManager->checkForNewMixes(yaml_url);

                // Start background downloads if no local mixes were found
                if (!configFilePath.empty()) {
                    ConfigFile config(configFilePath);
                    if (config.getAutoDownload()) {
                        startBackgroundDownloads();
                    }
                }
            }
            _backgroundTaskRunning.store(false);
        });
    }
}

void AutoVibezApp::handleMixControls(SDL_Event* event) {
    if (!_mixManagerInitialized)
        return;

    SDL_Keycode keycode = event->key.keysym.sym;

    switch (keycode) {
        case SDLK_r:
            // R: Random preset
            if (_presetManager) {
                _presetManager->randomPreset();
            }
            return;

        case SDLK_LEFT:
            // ←: Previous mix
            {
                Mix prevMix = _mixManager->getPreviousMix(_currentMix.id);
                if (!prevMix.id.empty()) {
                    _mixManager->downloadAndPlayMix(prevMix);
                    _currentMix = prevMix;
                }
            }
            return;

        case SDLK_RIGHT:
            // →: Next mix
            {
                Mix nextMix = _mixManager->getNextMix(_currentMix.id);
                if (!nextMix.id.empty()) {
                    _mixManager->downloadAndPlayMix(nextMix);
                    _currentMix = nextMix;
                }
            }
            return;

        case SDLK_UP:
            // ↑: Volume up
            {
                int currentVolume = _mixManager->getVolume();
                _mixManager->setVolume(currentVolume + 10, true);
                _volumeKeyPressed = true;
            }
            return;

        case SDLK_DOWN:
            // ↓: Volume down
            {
                int currentVolume = _mixManager->getVolume();
                _mixManager->setVolume(currentVolume - 10, true);
                _volumeKeyPressed = true;
            }
            return;

        case SDLK_m:
            // M: Mute/Unmute (universal media standard)
            if (_mixManagerInitialized) {
                int currentVolume = _mixManager->getVolume();
                if (currentVolume > 0) {
                    // Store current volume and mute
                    _previousVolume = currentVolume;
                    _mixManager->setVolume(0, true);
                } else {
                    // Unmute by restoring previous volume
                    _mixManager->setVolume(_previousVolume, true);
                }
            }
            return;

        case SDLK_1:
        case SDLK_2:
        case SDLK_3:
        case SDLK_4:
        case SDLK_5:

        case SDLK_f:
            // F: Toggle favorite
            if (!_currentMix.id.empty()) {
                _mixManager->toggleFavorite(_currentMix.id);
            }
            return;

        case SDLK_l:
            // L: Toggle mix table filter when help overlay is visible
            if (_helpOverlay && _helpOverlay->isVisible()) {
                _helpOverlay->toggleMixTableFilter();
            }
            return;
    }
}

void AutoVibezApp::startBackgroundDownloads() {
    if (!_mixManagerInitialized) {
        return;
    }

    // Download missing mixes in the background
    _mixManager->downloadMissingMixesBackground();
}

void AutoVibezApp::checkAndAutoPlayNext() {
    if (!_mixManagerInitialized || !_mixManager) {
        return;
    }

    // Check if music has stopped playing (not just paused)
    if (!_mixManager->isPlaying() && !_mixManager->isPaused()) {
        // Music has ended or stopped, play next random mix
        Mix nextMix = _mixManager->getSmartRandomMix(_currentMix.id, _mixManager->getCurrentGenre());
        if (!nextMix.id.empty()) {
            if (_mixManager->downloadAndPlayMix(nextMix)) {
                _currentMix = nextMix;
            } else {
                // Auto-play failed, try another mix
                nextMix = _mixManager->getSmartRandomMix(nextMix.id, _mixManager->getCurrentGenre());
                if (!nextMix.id.empty()) {
                    if (_mixManager->downloadAndPlayMix(nextMix)) {
                        _currentMix = nextMix;
                    }
                }
            }
        } else {
            // No more mixes available, try to get any mix
            nextMix = _mixManager->getRandomMix(_currentMix.id);
            if (!nextMix.id.empty()) {
                if (_mixManager->downloadAndPlayMix(nextMix)) {
                    _currentMix = nextMix;
                }
            }
        }
    }
}

void AutoVibezApp::autoPlayFromLocalDatabase() {
    if (!_mixManagerInitialized) {
        return;
    }

    // Try to play a mix from the local database with preferred genre
    Mix randomMix = _mixManager->getSmartRandomMix("", _mixManager->getCurrentGenre());

    if (!randomMix.id.empty()) {
        if (_mixManager->playMix(randomMix)) {
            _currentMix = randomMix;
        } else {
            // Play failed, try another mix
            randomMix = _mixManager->getSmartRandomMix(randomMix.id, _mixManager->getCurrentGenre());
            if (!randomMix.id.empty()) {
                if (_mixManager->playMix(randomMix)) {
                    _currentMix = randomMix;
                }
            }
        }
    } else {
        // No mixes with preferred genre, try any mix
        randomMix = _mixManager->getRandomMix(_currentMix.id);
        if (!randomMix.id.empty()) {
            if (_mixManager->playMix(randomMix)) {
                _currentMix = randomMix;
            }
        }
    }
}

}  // namespace Core
}  // namespace AutoVibez