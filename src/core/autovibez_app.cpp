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
#include "setup.hpp"
#include "mix_manager.hpp"
#include "mix_metadata.hpp"
#include "mix_downloader.hpp"
#include "config_manager.hpp"
#include "path_manager.hpp"

#include <vector>
#include <filesystem>
#include <set>
#include <thread>
#include <random>
#include <cstdlib>
#include <array>
#if defined _MSC_VER
#include <direct.h>
#else
#include <sys/stat.h>
#endif

using AutoVibez::Data::MixManager;

using AutoVibez::Data::Mix;
using AutoVibez::Data::ConfigFile;
using AutoVibez::UI::HelpOverlay;

namespace AutoVibez {
namespace Core {

AutoVibezApp::AutoVibezApp(SDL_GLContext glCtx, const std::string& presetPath, const std::string& texturePath, int audioDeviceIndex, bool showFps)
    : _openGlContext(glCtx)
    , _projectM(projectm_create())
    , _playlist(projectm_playlist_create(_projectM))
    , _selectedAudioDeviceIndex(audioDeviceIndex)
    , _mixManagerInitialized(false)
    , _hadMixesOnStartup(false) {
    projectm_get_window_size(_projectM, &_width, &_height);
    projectm_playlist_set_preset_switched_event_callback(_playlist, &AutoVibezApp::presetSwitchedEvent, static_cast<void*>(this));
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
void AutoVibezApp::stretchMonitors()
{
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
void AutoVibezApp::nextMonitor()
{
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
    }
    else
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_SetWindowFullscreen(_sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        _isFullScreen = true;
        if (_helpOverlay) {
            _helpOverlay->setFullscreenState(true);
        }
    }
}

void AutoVibezApp::scrollHandler(const SDL_Event* sdl_evt)
{
    // handle mouse scroll wheel - up++
    if (sdl_evt->wheel.y > 0)
    {
        _manualPresetChange = true;
        projectm_playlist_play_previous(_playlist, true);
        // Previous preset
        if (_playlist) {
            projectm_playlist_play_previous(_playlist, true);
            std::string preset_name = getActivePresetName();
            size_t last_slash = preset_name.find_last_of('/');
            if (last_slash != std::string::npos) {
                preset_name = preset_name.substr(last_slash + 1);
            }
            // Preset change notification removed - help overlay shows current preset
        }
    }
    // handle mouse scroll wheel - down--
    if (sdl_evt->wheel.y < 0)
    {
        _manualPresetChange = true;
        projectm_playlist_play_next(_playlist, true);
        // Next preset
        if (_playlist) {
            projectm_playlist_play_next(_playlist, true);
            std::string preset_name = getActivePresetName();
            size_t last_slash = preset_name.find_last_of('/');
            if (last_slash != std::string::npos) {
                preset_name = preset_name.substr(last_slash + 1);
            }
            // Preset change notification removed - help overlay shows current preset
        }
    }
}

void AutoVibezApp::keyHandler(SDL_Event* sdl_evt)
{
    if (!_mixManagerInitialized) {
        initMixManager();
    }
    
    // Handle mix controls first
    handleMixControls(sdl_evt);
    
    SDL_Keymod sdl_mod = (SDL_Keymod) sdl_evt->key.keysym.mod;
    SDL_Keycode sdl_keycode = sdl_evt->key.keysym.sym;

    // Left or Right Gui or Left Ctrl
    if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
    {
        keymod = true;
    }

    switch (sdl_keycode)
    {
        case SDLK_q:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // cmd/ctrl-q = quit
                done = true;
                return;
            }
            break;

        case SDLK_i:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                toggleAudioInput();
                return; // handled
            }
            break;

        case SDLK_s:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // command-s: [s]tretch monitors
                if (!this->stretch)
                {
                    stretchMonitors();
                    this->stretch = true;
                }
                else
                {
                    toggleFullScreen(); // else, just toggle full screen so we leave stretch mode.
                    this->stretch = false;
                }
                return; // handled
            }

        case SDLK_m:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // command-m: change [m]onitor
                nextMonitor();
                this->stretch = false;
                return;                // handled
            }

        case SDLK_f:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // command-f: fullscreen
                toggleFullScreen();
                this->stretch = false;
                return;                // handled
            }
            break;

        case SDLK_UP:
            if (_mixManagerInitialized && _mixManager->isPlaying()) {
            }
            break;

        case SDLK_DOWN:
            if (_mixManagerInitialized && _mixManager->isPlaying()) {
            }
            break;

        case SDLK_b:
            {
                float newSensitivity = getBeatSensitivity() + 0.1f;
                if (newSensitivity > 1.0f) newSensitivity = 1.0f;
                setBeatSensitivity(newSensitivity);
            }
            break;

        case SDLK_j:
            {
                float newSensitivity = getBeatSensitivity() - 0.1f;
                if (newSensitivity < 0.0f) newSensitivity = 0.0f;
                setBeatSensitivity(newSensitivity);
                // Beat sensitivity notification removed - help overlay shows current sensitivity
            }
            break;

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
            // SPACE: Random mix
            {
                Mix randomMix = _mixManager->getSmartRandomMix(_currentMix.id, _mixManager->getCurrentGenre());
                if (!randomMix.id.empty()) {
                    _mixManager->downloadAndPlayMix(randomMix);
                    _currentMix = randomMix;
                }
            }
            break;

        case SDLK_LEFTBRACKET:
            // [: Previous preset
            _manualPresetChange = true;
            projectm_playlist_play_previous(_playlist, true);
            // Preset change notification removed - help overlay shows current preset
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
                // Preset change notification removed - help overlay shows current preset
            }
            break;
            
        case SDLK_g:
            // G: Random mix in current genre
            if (_mixManagerInitialized) {
                // Check for modifier keys
                if (sdl_mod & KMOD_LSHIFT || sdl_mod & KMOD_RSHIFT) {
                    // Shift+G: Switch to random genre
                    std::string newGenre = _mixManager->getRandomGenre();
                    // Genre change notification removed - help overlay shows current genre
                    
                    // Play a random mix in the new genre
                    Mix genreMix = _mixManager->getRandomMixByGenre(newGenre, _currentMix.id);
                    if (!genreMix.id.empty()) {
                        _mixManager->downloadAndPlayMix(genreMix);
                        _currentMix = genreMix;
                    } else {
                        // Genre switch notification removed - too verbose for normal operation
                    }
                } else if (sdl_mod & KMOD_LCTRL || sdl_mod & KMOD_RCTRL) {
                    // Ctrl+G: Show available genres
                    auto genres = _mixManager->getAvailableGenres();
                    // Genre list notification removed - help overlay shows current genre
                } else {
                    // G: Random mix in current mix's genre
                    if (!_currentMix.id.empty() && !_currentMix.genre.empty()) {
                        Mix genreMix = _mixManager->getRandomMixByGenre(_currentMix.genre, _currentMix.id);
                        if (!genreMix.id.empty()) {
                            _mixManager->downloadAndPlayMix(genreMix);
                            _currentMix = genreMix;
                        } else {
                            // Genre switch notification removed - too verbose for normal operation
                        }
                    } else {
                        // Genre switch notification removed - too verbose for normal operation
                    }
                }
            }
            break;
    }
}



void AutoVibezApp::resizeWindow(unsigned int width_, unsigned int height_)
{
    _width = width_;
    _height = height_;

    // Hide cursor if window size equals desktop size
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0)
    {
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
            case SDL_MOUSEWHEEL:
                handleMouseWheelEvent(evt);
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
    handleMouseDragEvent();
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

void AutoVibezApp::handleMouseWheelEvent(const SDL_Event& evt) {
    scrollHandler(&evt);
}

void AutoVibezApp::handleKeyDownEvent(const SDL_Event& evt) {
    keyHandler(const_cast<SDL_Event*>(&evt));
}

void AutoVibezApp::handleKeyUpEvent(const SDL_Event& evt) {
    if (_volumeKeyPressed && (evt.key.keysym.sym == SDLK_UP || evt.key.keysym.sym == SDLK_DOWN)) {
        // Volume notification removed - help overlay shows current volume
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

void AutoVibezApp::handleMouseDragEvent() {
    // Mouse drag handling removed - touch functionality disabled
}



void AutoVibezApp::renderFrame()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_opengl_render_frame(_projectM);

    // Render overlays
    renderHelpOverlay();

    SDL_GL_SwapWindow(_sdlWindow);
}

void AutoVibezApp::initialize(SDL_Window* window, const bool _renderToTexture)
{
    (void)_renderToTexture; // Parameter not used in current implementation
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
    if (!_helpOverlay) return;
    
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
    const char* deviceName = SDL_GetAudioDeviceName(_selectedAudioDeviceIndex, SDL_TRUE);
    if (deviceName) {
        _helpOverlay->setAudioDevice(deviceName);
    }
    
    // Update beat sensitivity
    _helpOverlay->setBeatSensitivity(getBeatSensitivity());
    
    // Update mix table data
    if (_mixManager) {
        auto mixes = _mixManager->getAllMixes();
        _helpOverlay->setMixTableData(mixes);
    }
}


std::string AutoVibezApp::getActivePresetName()
{
    unsigned int index = projectm_playlist_get_position(_playlist);
    if (index)
    {
        auto presetName = projectm_playlist_item(_playlist, index);
        std::string presetNameString(presetName);
        projectm_playlist_free_string(presetName);
        return presetNameString;
    }
    return {};
}

void AutoVibezApp::presetSwitchedEvent(bool isHardCut, unsigned int index, void* context)
{
    auto app = reinterpret_cast<AutoVibezApp*>(context);
    auto presetName = projectm_playlist_item(app->_playlist, index);
    
    app->_presetName = presetName;
    
    // Only output for automatic changes (not manual ones)
    if (!app->_manualPresetChange) {
        // Extract just the filename for display
        std::string preset_name = presetName;
        size_t last_slash = preset_name.find_last_of('/');
        if (last_slash != std::string::npos) {
            preset_name = preset_name.substr(last_slash + 1);
        }
        
        // Use complete reinitialization for automatic changes to ensure clean state
        if (app->_helpOverlay) {
            app->_helpOverlay->triggerCompleteReinitialization();
        }
    } else {
        // Use immediate texture rebind for manual changes
        if (app->_helpOverlay) {
            app->_helpOverlay->triggerTextureRebind();
        }
    }
    
    app->_manualPresetChange = false;
    
    projectm_playlist_free_string(presetName);
    app->UpdateWindowTitle();
}

projectm_handle AutoVibezApp::projectM()
{
    return _projectM;
}





float AutoVibezApp::getBeatSensitivity() const {
    return projectm_get_beat_sensitivity(_projectM);
}

void AutoVibezApp::setBeatSensitivity(float sensitivity) {
    projectm_set_beat_sensitivity(_projectM, sensitivity);
}

void AutoVibezApp::UpdateWindowTitle()
{
    std::string title = "AutoVibez";
    if (projectm_get_preset_locked(_projectM))
    {
        title.append(" [locked]");
    }
    
    SDL_SetWindowTitle(_sdlWindow, title.c_str());
}







void AutoVibezApp::cycleAudioDevice()
{
    // Calculate next device index like the original - will wrap around to default capture device (-1)
    int nextAudioDeviceId = ((_selectedAudioDeviceIndex + 2) % (SDL_GetNumAudioDevices(SDL_TRUE) + 1)) - 1;
    
    // Start recording with new device
    _selectedAudioDeviceIndex = nextAudioDeviceId;
    if (initializeAudioInput()) {
        beginAudioCapture();
    }
}







// Mix management methods
void AutoVibezApp::initMixManager()
{
    if (_mixManagerInitialized) return;
    
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
        // Mix manager initialization error notification removed - too verbose for normal operation
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
            // Audio device notification removed - help overlay shows current device
        } else {
            // Audio device notification removed - help overlay shows current device
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

void AutoVibezApp::handleMixControls(SDL_Event* event)
{
    if (!_mixManagerInitialized) return;
    
    SDL_Keycode keycode = event->key.keysym.sym;
    
    switch (keycode) {
        case SDLK_r:
            // R: Random preset
            if (_presetManager) {
                _presetManager->randomPreset();
            }
            return;
            
        case SDLK_n:
            // N: Next mix
            {
                Mix nextMix = _mixManager->getNextMix(_currentMix.id);
                if (!nextMix.id.empty()) {
                    _mixManager->downloadAndPlayMix(nextMix);
                    _currentMix = nextMix;
                }
            }
            return;
            
        case SDLK_p:
            // P: Pause/Resume
            _mixManager->togglePause();
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
            
        case SDLK_v:
            // V: List favorite mixes (handled by help overlay)
            return;
    }
}

void AutoVibezApp::autoPlayOrDownload()
{
    if (!_mixManagerInitialized) {
        // Auto-play initialization error notification removed - too verbose for normal operation
        return;
    }
    
    // Step 1: Check for existing downloaded mixes and play one, prioritizing preferred genre
    auto downloadedMixes = _mixManager->getDownloadedMixes();
    if (!downloadedMixes.empty()) {
        // First try to play a mix from preferred genre
        Mix randomMix = _mixManager->getRandomMixByGenre(_mixManager->getCurrentGenre(), _currentMix.id);
        if (randomMix.id.empty()) {
            // If no mixes in preferred genre, try a favorite mix
            randomMix = _mixManager->getRandomFavoriteMix(_currentMix.id);
        }
        if (randomMix.id.empty()) {
            // If no favorites, fall back to any random mix
            randomMix = _mixManager->getRandomMix(_currentMix.id);
        }
        if (!randomMix.id.empty()) {
            if (_mixManager->playMix(randomMix)) {
                _currentMix = randomMix;
            } else {
                // Play failed notification removed - too verbose for normal operation
            }
        }
        
        // Start background download of remaining mixes
        startBackgroundDownloads();
        return;
    }
    
    // Step 2: No existing mixes, download and play one from preferred genre
    // Download notification removed - too verbose for normal operation
    
    // Try to get a mix from the preferred genre first
    Mix randomMix = _mixManager->getRandomAvailableMixByGenre(_mixManager->getCurrentGenre());
    if (randomMix.id.empty()) {
        // If no mixes in preferred genre, fall back to any random mix
        randomMix = _mixManager->getRandomAvailableMix();
    }
    if (randomMix.id.empty()) {
        // If no mixes available notification removed - too verbose for normal operation
        return;
    }
    
    // Download notification removed - too verbose for normal operation
    
    if (_mixManager->downloadAndAnalyzeMix(randomMix)) {
        // Get the updated mix with complete metadata from database
        Mix updatedMix = _mixManager->getMixById(randomMix.id);
        if (!updatedMix.id.empty()) {
            // Now play the analyzed mix
            if (_mixManager->playMix(updatedMix)) {
                _currentMix = updatedMix;
            } else {
                // Play failed notification removed - too verbose for normal operation
            }
        }
        
        // Start background download of remaining mixes
        startBackgroundDownloads();
    } else {
        // Download failed notification removed - too verbose for normal operation
    }
}

void AutoVibezApp::autoDownloadRandomMix()
{
    if (!_mixManagerInitialized) {
        // Auto-download initialization error notification removed - too verbose for normal operation
        return;
    }
    
    // First try to get a random mix from available (not yet downloaded) mixes in preferred genre
    Mix randomMix = _mixManager->getRandomAvailableMixByGenre(_mixManager->getCurrentGenre(), _currentMix.id);
    if (randomMix.id.empty()) {
        // If no available mixes in preferred genre, try any available mix
        randomMix = _mixManager->getRandomAvailableMix(_currentMix.id);
    }
    if (randomMix.id.empty()) {
        // If no available mixes, try database (already downloaded) - prioritize favorites
        randomMix = _mixManager->getRandomFavoriteMix(_currentMix.id);
    }
    if (randomMix.id.empty()) {
        // If no favorites, try database (already downloaded) in preferred genre
        randomMix = _mixManager->getRandomMixByGenre(_mixManager->getCurrentGenre(), _currentMix.id);
    }
    if (randomMix.id.empty()) {
        // If no mixes in preferred genre, try any random mix from database
        randomMix = _mixManager->getRandomMix(_currentMix.id);
    }
    if (randomMix.id.empty()) {
        // No mixes available for auto-download notification removed - too verbose for normal operation
        return;
    }
    
    // Auto-download notification removed - too verbose for normal operation
    
    // Use the new download-first approach
    if (_mixManager->downloadAndAnalyzeMix(randomMix)) {
        // Auto-download success notification removed - too verbose for normal operation
        
        // Get the updated mix with complete metadata from database
        Mix updatedMix = _mixManager->getMixById(randomMix.id);
        if (!updatedMix.id.empty()) {
            // Now play the analyzed mix
            if (_mixManager->playMix(updatedMix)) {
                _currentMix = updatedMix; // Use the updated mix with metadata
                // Auto-play success notification removed - too verbose for normal operation
            } else {
                // Auto-play failed notification removed - too verbose for normal operation
            }
        } else {
            // Database retrieval error notification removed - too verbose for normal operation
        }
    } else {
        // Auto-download failed notification removed - too verbose for normal operation
    }
}

void AutoVibezApp::startBackgroundDownloads()
{
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
            // Auto-play next mix notification removed - too verbose for normal operation
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

void AutoVibezApp::autoPlayFromLocalDatabase()
{
    if (!_mixManagerInitialized) {
        // Auto-play initialization error notification removed - too verbose for normal operation
        return;
    }

    // Try to play a mix from the local database with preferred genre
    Mix randomMix = _mixManager->getSmartRandomMix("", _mixManager->getCurrentGenre());
    
    if (!randomMix.id.empty()) {
        if (_mixManager->playMix(randomMix)) {
            _currentMix = randomMix;
            // Auto-play success notification removed - too verbose for normal operation
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

} // namespace Core
} // namespace AutoVibez