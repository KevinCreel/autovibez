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
#include "logger.hpp"
#include "console_output.hpp"
#include "mix_manager.hpp"
#include "mix_display.hpp"
#include "config_manager.hpp"

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
using AutoVibez::Data::MixDisplay;
using AutoVibez::Data::Mix;
using AutoVibez::Audio::AudioManager;
using AutoVibez::Data::ConfigFile;
using AutoVibez::UI::HelpOverlay;

namespace AutoVibez {
namespace Core {

AutoVibezApp::AutoVibezApp(SDL_GLContext glCtx, const std::string& presetPath, const std::string& texturePath, int audioDeviceIndex, bool showFps)
    : _openGlContext(glCtx)
    , _projectM(projectm_create())
    , _playlist(projectm_playlist_create(_projectM))
    , _texturePath(texturePath)
    , _showFps(showFps)
    , _selectedAudioDeviceIndex(audioDeviceIndex)
{
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

AutoVibezApp::~AutoVibezApp()
{
    // Wait for background task to complete if it's running
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
    if (displayCount >= 2)
    {
        std::vector<SDL_Rect> displayBounds;
        for (int i = 0; i < displayCount; i++)
        {
            displayBounds.push_back(SDL_Rect());
            SDL_GetDisplayBounds(i, &displayBounds.back());
        }

        int mostXLeft = 0;
        int mostXRight = 0;
        int mostYUp = 0;
        int mostYDown = 0;

        for (int i = 0; i < displayCount; i++)
        {
            if (displayBounds[i].x < mostXLeft)
            {
                mostXLeft = displayBounds[i].x;
            }
            if ((displayBounds[i].x + displayBounds[i].w) > mostXRight)
            {
                mostXRight = displayBounds[i].x + displayBounds[i].w;
            }
        }
        for (int i = 0; i < displayCount; i++)
        {
            if (displayBounds[i].y < mostYUp)
            {
                mostYUp = displayBounds[i].y;
            }
            if ((displayBounds[i].y + displayBounds[i].h) > mostYDown)
            {
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
    if (displayCount >= 2)
    {
        std::vector<SDL_Rect> displayBounds;
        int nextWindow = currentWindowIndex + 1;
        if (nextWindow >= displayCount)
        {
            nextWindow = 0;
        }

        for (int i = 0; i < displayCount; i++)
        {
            displayBounds.push_back(SDL_Rect());
            SDL_GetDisplayBounds(i, &displayBounds.back());
        }
        SDL_SetWindowPosition(_sdlWindow, displayBounds[nextWindow].x, displayBounds[nextWindow].y);
        SDL_SetWindowSize(_sdlWindow, displayBounds[nextWindow].w, displayBounds[nextWindow].h);
    }
}

void AutoVibezApp::syncFullscreenState()
{
    Uint32 flags = SDL_GetWindowFlags(_sdlWindow);
    _isFullScreen = (flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void AutoVibezApp::toggleFullScreen()
{
    if (_isFullScreen)
    {
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
        std::string preset_name = getActivePresetName();
        size_t last_slash = preset_name.find_last_of('/');
        if (last_slash != std::string::npos) {
            preset_name = preset_name.substr(last_slash + 1);
        }
        ConsoleOutput::output("⏮️  Previous preset: %s", preset_name.c_str());
    }
    // handle mouse scroll wheel - down--
    if (sdl_evt->wheel.y < 0)
    {
        _manualPresetChange = true;
        projectm_playlist_play_next(_playlist, true);
        std::string preset_name = getActivePresetName();
        size_t last_slash = preset_name.find_last_of('/');
        if (last_slash != std::string::npos) {
            preset_name = preset_name.substr(last_slash + 1);
        }
        ConsoleOutput::output("⏭️  Next preset: %s", preset_name.c_str());
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

    // handle keyboard input (for our app first, then projectM)
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
                // Stereo requires fullscreen
#if !STEREOSCOPIC_SBS
                if (!this->stretch)
                { // if stretching is not already enabled, enable it.
                    stretchMonitors();
                    this->stretch = true;
                }
                else
                {
                    toggleFullScreen(); // else, just toggle full screen so we leave stretch mode.
                    this->stretch = false;
                }
#endif
                return; // handled
            }

        case SDLK_m:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // command-m: change [m]onitor
                // Stereo requires fullscreen
#if !STEREOSCOPIC_SBS
                nextMonitor();
#endif
                this->stretch = false; // if we are switching monitors, ensure we disable monitor stretching.
                return;                // handled
            }

        case SDLK_f:
            if (sdl_mod & KMOD_LGUI || sdl_mod & KMOD_RGUI || sdl_mod & KMOD_LCTRL)
            {
                // command-f: fullscreen
                // Stereo requires fullscreen
#if !STEREOSCOPIC_SBS
                toggleFullScreen();
#endif
                this->stretch = false; // if we are toggling fullscreen, ensure we disable monitor stretching.
                return;                // handled
            }
            break;

        case SDLK_UP:
            // Handle volume control for mixes
            if (_mixManagerInitialized && _mixManager->isPlaying()) {
                // Volume control handled in handleMixControls
            }
            break;

        case SDLK_DOWN:
            // Handle volume control for mixes
            if (_mixManagerInitialized && _mixManager->isPlaying()) {
                // Volume control handled in handleMixControls
            }
            break;

        case SDLK_b:
            // B: Increase beat sensitivity
            {
                float currentSensitivity = projectm_get_beat_sensitivity(_projectM);
                float newSensitivity = currentSensitivity + 0.1f;
                if (newSensitivity > 5.0f) newSensitivity = 5.0f; // Cap at 5.0
                projectm_set_beat_sensitivity(_projectM, newSensitivity);
                ConsoleOutput::output("🎵 Beat Sensitivity: %.1f", newSensitivity);
                ConsoleOutput::output("");
            }
            break;

        case SDLK_j:
            // J: Decrease beat sensitivity
            {
                float currentSensitivity = projectm_get_beat_sensitivity(_projectM);
                float newSensitivity = currentSensitivity - 0.1f;
                if (newSensitivity < 0.0f) newSensitivity = 0.0f;
                projectm_set_beat_sensitivity(_projectM, newSensitivity);
                ConsoleOutput::output("🎵 Beat Sensitivity: %.1f", newSensitivity);
                ConsoleOutput::output("");
            }
            break;

        case SDLK_F11:
            // F11: toggle fullscreen
            toggleFullScreen();
            break;

        case SDLK_h:
            // H: toggle help overlay
            toggleHelp();
            // Also toggle simple UI
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
                    _showMixStatus = true;
                    _mixStatusDisplayTime = 300; // Show for 5 seconds
                    _mixInfoDisplayed = false;
                }
            }
            break;
            

            
        case SDLK_LEFTBRACKET:
            // [: Previous preset
            _manualPresetChange = true;
            projectm_playlist_play_previous(_playlist, true);
            {
                std::string preset_name = getActivePresetName();
                size_t last_slash = preset_name.find_last_of('/');
                if (last_slash != std::string::npos) {
                    preset_name = preset_name.substr(last_slash + 1);
                }
                ConsoleOutput::output("⏮️  Previous preset: %s", preset_name.c_str());
            }
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
                ConsoleOutput::output("⏭️  Next preset: %s", preset_name.c_str());
            }
            break;
            
        case SDLK_g:
            // G: Random mix in current genre
            if (_mixManagerInitialized) {
                // Check for modifier keys
                if (sdl_mod & KMOD_LSHIFT || sdl_mod & KMOD_RSHIFT) {
                    // Shift+G: Switch to random genre
                    std::string newGenre = _mixManager->getRandomGenre();
                    ConsoleOutput::output("🎼 Switched to genre: %s", newGenre.c_str());
                    ConsoleOutput::output("");
                    
                    // Play a random mix in the new genre
                    Mix genreMix = _mixManager->getRandomMixByGenre(newGenre);
                    if (!genreMix.id.empty()) {
                        _mixManager->downloadAndPlayMix(genreMix);
                        _currentMix = genreMix;
                        _showMixStatus = true;
                        _mixStatusDisplayTime = 300;
                        _mixInfoDisplayed = false;
                    }
                } else if (sdl_mod & KMOD_LCTRL || sdl_mod & KMOD_RCTRL) {
                    // Ctrl+G: Show available genres
                    auto genres = _mixManager->getAvailableGenres();
                    ConsoleOutput::output("🎼 Available genres (%zu):", genres.size());
                    for (const auto& genre : genres) {
                        // Create a local copy for Title Case display only
                        std::string display_genre = genre;
                        bool capitalize = true;
                        for (char& c : display_genre) {
                            if (capitalize && std::isalpha(c)) {
                                c = std::toupper(c);
                                capitalize = false;
                            } else if (std::isspace(c) || c == '-') {
                                capitalize = true;
                            } else if (std::isalpha(c)) {
                                c = std::tolower(c);
                            }
                        }
                        ConsoleOutput::output("  • %s", display_genre.c_str());
                    }
                    // Create a local copy for current genre Title Case display only
                    std::string display_current_genre = _currentMix.genre;
                    bool capitalize = true;
                    for (char& c : display_current_genre) {
                        if (capitalize && std::isalpha(c)) {
                            c = std::toupper(c);
                            capitalize = false;
                        } else if (std::isspace(c) || c == '-') {
                            capitalize = true;
                        } else if (std::isalpha(c)) {
                            c = std::tolower(c);
                        }
                    }
                    ConsoleOutput::output("🎼 Current genre: %s", display_current_genre.c_str());
                    ConsoleOutput::output("");
                } else {
                    // G: Random mix in current mix's genre
                    if (!_currentMix.id.empty() && !_currentMix.genre.empty()) {
                        Mix genreMix = _mixManager->getRandomMixByGenre(_currentMix.genre);
                        if (!genreMix.id.empty()) {
                            _mixManager->downloadAndPlayMix(genreMix);
                            _currentMix = genreMix;
                            _showMixStatus = true;
                            _mixStatusDisplayTime = 300;
                            _mixInfoDisplayed = false;
                        } else {
                            ConsoleOutput::output("❌ No other mixes found in genre: %s", _currentMix.genre.c_str());
                            ConsoleOutput::output("");
                        }
                    } else {
                        ConsoleOutput::output("❌ No mix currently playing or no genre available");
                        ConsoleOutput::output("");
                    }
                }
            }
            break;
            


    }
}

void AutoVibezApp::addFakePCM()
{
    int i;
    std::array<int16_t, 2 * 512> pcm_data;
    /** Produce some fake PCM data to stuff into projectM */
    for (i = 0; i < 512; i++)
    {
        if (i % 2 == 0)
        {
            pcm_data[2 * i] = (float) (rand() / ((float) RAND_MAX) * (pow(2, 14)));
            pcm_data[2 * i + 1] = (float) (rand() / ((float) RAND_MAX) * (pow(2, 14)));
        }
        else
        {
            pcm_data[2 * i] = (float) (rand() / ((float) RAND_MAX) * (pow(2, 14)));
            pcm_data[2 * i + 1] = (float) (rand() / ((float) RAND_MAX) * (pow(2, 14)));
        }
        if (i % 2 == 1)
        {
            pcm_data[2 * i] = -pcm_data[2 * i];
            pcm_data[2 * i + 1] = -pcm_data[2 * i + 1];
        }
    }

    /** Add the waveform data */
    projectm_pcm_add_int16(_projectM, pcm_data.data(), 512, PROJECTM_STEREO);
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
        ConsoleOutput::output("🔊 Volume: %d%%", _mixManager->getVolume());
        _volumeKeyPressed = false;
    }
}

void AutoVibezApp::handleMouseButtonDownEvent(const SDL_Event& evt) {
    if (evt.button.button == SDL_BUTTON_LEFT) {
        if (!mouseDown) {
            int mousex, mousey;
            SDL_GetMouseState(&mousex, &mousey);
            float mousexscale = (mousex / (float)_width);
            float mouseyscale = ((_height - mousey) / (float)_height);
            handleTouch(mousexscale, mouseyscale, 0);
            mouseDown = true;
        }
    } else if (evt.button.button == SDL_BUTTON_RIGHT) {
        mouseDown = false;
        if (keymod) {
            destroyAllTouches();
            keymod = false;
            return;
        }
        int mousex, mousey;
        SDL_GetMouseState(&mousex, &mousey);
        float mousexscale = (mousex / (float)_width);
        float mouseyscale = ((_height - mousey) / (float)_height);
        destroyTouch(mousexscale, mouseyscale);
    }
}

void AutoVibezApp::handleMouseButtonUpEvent(const SDL_Event& evt) {
    mouseDown = false;
}

void AutoVibezApp::handleQuitEvent(const SDL_Event&) {
    done = true;
}

void AutoVibezApp::handleMouseDragEvent() {
    if (mouseDown) {
        int mousex, mousey;
        SDL_GetMouseState(&mousex, &mousey);
        float mousexscale = (mousex / (float)_width);
        float mouseyscale = ((_height - mousey) / (float)_height);
        handleTouchDrag(mousexscale, mouseyscale, 0);
    }
}

// This touches the screen to generate a waveform at X / Y.
void AutoVibezApp::handleTouch(float x, float y, int pressure, int touchtype)
{
#ifdef PROJECTM_TOUCH_ENABLED
    projectm_touch(_projectM, x, y, pressure, static_cast<projectm_touch_type>(touchtype));
#else
    (void)x; (void)y; (void)pressure; (void)touchtype; // Touch handling disabled
#endif
}

// This moves the X Y of your existing waveform that was generated by a touch (only if you held down your click and dragged your mouse around).
void AutoVibezApp::handleTouchDrag(float x, float y, int pressure)
{
    projectm_touch_drag(_projectM, x, y, pressure);
}

// Remove waveform at X Y
void AutoVibezApp::destroyTouch(float x, float y)
{
    projectm_touch_destroy(_projectM, x, y);
}

// Remove all waveforms
void AutoVibezApp::destroyAllTouches()
{
    projectm_touch_destroy_all(_projectM);
}

void AutoVibezApp::renderFrame()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_opengl_render_frame(_projectM);

    // Render overlays
    printHelpMenu();
    renderFpsCounter();
    displayMixStatus();
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
        _helpOverlay->render();
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
        
                            ConsoleOutput::output("🎨 Preset: %s", preset_name.c_str());
        
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

void AutoVibezApp::setFps(size_t fps)
{
    _fps = fps;
}

size_t AutoVibezApp::fps() const {
    return _fps;
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

// New feature implementations
void AutoVibezApp::toggleHelp()
{
    _showHelp = !_showHelp;
    if (_showHelp) {
        printHelpMenu();
    }
}



void AutoVibezApp::cycleAudioDevice()
{
    if (_numAudioDevices > 0) {
        _selectedAudioDeviceIndex = (_selectedAudioDeviceIndex + 1) % _numAudioDevices;
        
        // Get the device name for feedback
        const char* deviceName = SDL_GetAudioDeviceName(_selectedAudioDeviceIndex, SDL_TRUE);
        if (deviceName) {
            ConsoleOutput::output("🎚️  Switched to audio device: %s", deviceName);
        } else {
            ConsoleOutput::output("🎚️  Switched to audio device: %d", _selectedAudioDeviceIndex);
        }
        
        // Reopen audio with new device
        endAudioCapture();
        _curAudioDevice = _selectedAudioDeviceIndex;
        _selectedAudioDevice = _selectedAudioDeviceIndex;  // Set both variables
        initializeAudioInput();  // Use initializeAudioInput directly instead of openAudioInput
        beginAudioCapture();
    } else {
        ConsoleOutput::output("🎚️  No audio devices available");
    }
}

void AutoVibezApp::printHelpMenu()
{
    if (!_showHelp) return;
    static bool helpShown = false;
    if (!helpShown) {
        const char* color_cyan = "\033[36m";
        const char* color_green = "\033[32m";
        const char* color_yellow = "\033[33m";
        const char* color_purple = "\033[35m";
        const char* color_reset = "\033[0m";
        
        ConsoleOutput::output("\n");
        ConsoleOutput::output("%s✨🎵 %sAutoVibez Controls%s 🎵✨%s", color_purple, color_green, color_reset, color_purple);
        ConsoleOutput::output("\n");
        ConsoleOutput::output("%s🎧 %sMix Management:%s", "\033[32m", color_green, color_reset);
        ConsoleOutput::output("%sN%s     - Play next mix", "\033[32m", color_reset);
        ConsoleOutput::output("%sF%s     - Toggle favorite", "\033[32m", color_reset);
        ConsoleOutput::output("%sV%s     - List favorite mixes", "\033[32m", color_reset);
        ConsoleOutput::output("%sL%s     - List available mixes", "\033[32m", color_reset);
        ConsoleOutput::output("%sG%s     - Play random mix in current genre", "\033[32m", color_reset);
        ConsoleOutput::output("%sShift+G%s - Switch to random genre", "\033[32m", color_reset);
        ConsoleOutput::output("%sCtrl+G%s  - Show available genres", "\033[32m", color_reset);
        ConsoleOutput::output("%sSPACE%s  - Load random mix", "\033[32m", color_reset);
        ConsoleOutput::output("\n");
        ConsoleOutput::output("%s🎚️  %sAudio Controls:%s", "\033[34m", color_green, color_reset);
        ConsoleOutput::output("%sP%s     - Pause/Resume playback", "\033[34m", color_reset);
        ConsoleOutput::output("%s↑/↓%s   - Volume up/down", "\033[34m", color_reset);
        ConsoleOutput::output("%sTab%s   - Cycle through audio devices", "\033[34m", color_reset);
        ConsoleOutput::output("\n");
        ConsoleOutput::output("%s🌈 %sVisualizer Controls:%s", color_yellow, color_green, color_reset);
        ConsoleOutput::output("%sH%s     - Toggle this help overlay", color_yellow, color_reset);
        ConsoleOutput::output("%sF11%s   - Toggle fullscreen mode", color_yellow, color_reset);
        ConsoleOutput::output("%sR%s     - Load random preset", color_yellow, color_reset);
        ConsoleOutput::output("%s[ / ]%s  - Previous/Next preset", color_yellow, color_reset);
        ConsoleOutput::output("%sB / J%s  - Increase/Decrease beat sensitivity", color_yellow, color_reset);
        ConsoleOutput::output("%sMouse Wheel%s - Next/Prev preset", color_yellow, color_reset);
        ConsoleOutput::output("\n");
        ConsoleOutput::output("%s⚙️  %sApplication:%s", "\033[31m", color_green, color_reset);
        ConsoleOutput::output("%sCtrl+Q%s - Quit application", "\033[31m", color_reset);
        ConsoleOutput::output("\n");
        helpShown = true;
    }
}

void AutoVibezApp::renderFpsCounter()
{
    if (!_showFps) return;
    
    // Simple FPS display - in a real implementation you'd render this on screen
    static int frameCount = 0;
    static Uint32 lastTime = SDL_GetTicks();
    frameCount++;
    
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastTime >= 1000) { // Update every second
        float current_fps = (float)frameCount * 1000.0f / (currentTime - lastTime);
        ConsoleOutput::output("FPS: %.1f", current_fps);
        frameCount = 0;
        lastTime = currentTime;
    }
}

// Get XDG data directory for autovibez (cross-platform)
std::string AutoVibezApp::getDataDirectory() {
    return PathManager::getDataDirectory();
}

// Get XDG config directory for autovibez (cross-platform)
std::string AutoVibezApp::getConfigDirectory() {
    return PathManager::getConfigDirectory();
}

// Get XDG cache directory for autovibez (cross-platform)
std::string AutoVibezApp::getCacheDirectory() {
    return PathManager::getCacheDirectory();
}

// Get XDG state directory for autovibez (cross-platform)
std::string AutoVibezApp::getStateDirectory() {
    std::string state_dir;
    
#ifdef _WIN32
    // Windows: Use %LOCALAPPDATA%/autovibez/state
    const char* localappdata = std::getenv("LOCALAPPDATA");
    if (localappdata) {
        state_dir = std::string(localappdata) + "/autovibez/state";
    } else {
        state_dir = "state"; // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Application Support/autovibez/state
    const char* home = std::getenv("HOME");
    if (home) {
        state_dir = std::string(home) + "/Library/Application Support/autovibez/state";
    } else {
        state_dir = "state"; // Fallback
    }
#else
    // Linux/Unix: Use XDG Base Directory Specification
    const char* xdg_state_home = std::getenv("XDG_STATE_HOME");
    if (xdg_state_home && strlen(xdg_state_home) > 0) {
        state_dir = std::string(xdg_state_home) + "/autovibez";
    } else {
        // Fallback to default XDG location
        const char* home = std::getenv("HOME");
        if (home) {
            state_dir = std::string(home) + "/.local/state/autovibez";
        } else {
            state_dir = "state"; // Last resort fallback
        }
    }
#endif
    
    // Create directory if it doesn't exist
    if (!std::filesystem::exists(state_dir)) {
        std::filesystem::create_directories(state_dir);
    }
    
    return state_dir;
}

// Get XDG assets directory for autovibez (cross-platform)
std::string AutoVibezApp::getAssetsDirectory() {
    return PathManager::getAssetsDirectory();
}

// Mix management methods
void AutoVibezApp::initMixManager()
{
    if (_mixManagerInitialized) return;
    
    std::string data_dir = getDataDirectory();
    std::string db_path = getStateDirectory() + "/autovibez_mixes.db";
    std::string cache_dir = getCacheDirectory() + "/mix_cache";
    
    _mixManager = std::make_unique<MixManager>(db_path, cache_dir);
    _mixUI = std::make_unique<MixDisplay>();
    
    if (!_mixManager->initialize()) {
        ConsoleOutput::output("❌ Failed to initialize mix manager: %s", _mixManager->getLastError().c_str());
        return;
    }
    
    // Load configuration
    std::string configFilePath = findConfigFile();
    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);
        
        // Set initial genre from config
        std::string preferred_genre = config.getPreferredGenre();
        if (!preferred_genre.empty()) {
            ConsoleOutput::output("⚙️  Setting preferred genre from config: '%s'", preferred_genre.c_str());
        } else {
            ConsoleOutput::output("⚙️  No preferred genre set in config - will use random genres");
        }
        _mixManager->setCurrentGenre(preferred_genre);
        
        // Show current audio device
        int audioDeviceIndex = config.getAudioDeviceIndex();
        const char* deviceName = SDL_GetAudioDeviceName(audioDeviceIndex, SDL_TRUE);
        if (deviceName) {
            ConsoleOutput::output("🎚️  Using audio device: %s", deviceName);
        } else {
            ConsoleOutput::output("🎚️  Using audio device: %d", audioDeviceIndex);
        }
    }
    
    // Load mix metadata from YAML
    std::string yaml_url;
    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);
        yaml_url = config.getMixesUrl();
    }
    
    if (yaml_url.empty()) {
        ConsoleOutput::output("⚠️  No mixes_url configured, skipping mix metadata loading");
        return;
    }
    
    if (!_mixManager->loadMixMetadata(yaml_url)) {
        ConsoleOutput::output("❌ Failed to load mix metadata: %s", _mixManager->getLastError().c_str());
        return;
    }
    
    // Check for new mixes from remote YAML in background (non-blocking)
    // This will download new mixes but not block startup
    if (!_backgroundTaskRunning.load()) {
        _backgroundTaskRunning.store(true);
        _backgroundTask = std::async(std::launch::async, [this, yaml_url]() {
            std::lock_guard<std::mutex> lock(_mixManagerMutex);
            if (_mixManager) {
                _mixManager->checkForNewMixes(yaml_url);
            }
            _backgroundTaskRunning.store(false);
        });
    }
    
    _mixManagerInitialized = true;
    
    // Auto-play functionality: play existing mix or download and play
    // (moved here after mix manager is fully initialized)
    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);
        if (config.getAutoDownload()) {
            autoPlayOrDownload();
        }
    }
}

void AutoVibezApp::handleMixControls(SDL_Event* event)
{
    if (!_mixManagerInitialized) return;
    
    SDL_Keycode keycode = event->key.keysym.sym;
    
    switch (keycode) {
        case SDLK_r:
            // R: Random preset
            {
                uint32_t preset_count = projectm_playlist_size(_playlist);
                if (preset_count > 0) {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<uint32_t> dis(0, preset_count - 1);
                    uint32_t random_index = dis(gen);
                    projectm_playlist_set_position(_playlist, random_index, true);
                    std::string preset_name = getActivePresetName();
                    size_t last_slash = preset_name.find_last_of('/');
                    if (last_slash != std::string::npos) {
                        preset_name = preset_name.substr(last_slash + 1);
                    }
                    ConsoleOutput::output("🎨 Loaded random preset: %s", preset_name.c_str());
                    ConsoleOutput::output("🎨 Loaded random preset: %s", preset_name.c_str());
                }
            }
            return;
            
        case SDLK_n:
            // N: Next mix
            {
                Mix nextMix = _mixManager->getNextMix(_currentMix.id);
                if (!nextMix.id.empty()) {
                    _mixManager->downloadAndPlayMix(nextMix);
                    _currentMix = nextMix;
                    _showMixStatus = true;
                    _mixStatusDisplayTime = 300;
                    _mixInfoDisplayed = false;
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
                            ConsoleOutput::output("❤️  Toggled favorite for %s", _currentMix.title.c_str());
            }
            return;
            
        case SDLK_l:
            // L: List downloaded mixes
            {
                auto downloadedMixes = _mixManager->getDownloadedMixes();
                const char* color_cyan = "\033[36m";
                const char* color_yellow = "\033[33m";
                const char* color_green = "\033[32m";
                const char* color_magenta = "\033[35m";
                const char* color_reset = "\033[0m";
                
                ConsoleOutput::output("📋 Downloaded mixes (%zu):", downloadedMixes.size());
                for (const auto& mix : downloadedMixes) {
                    ConsoleOutput::output("  - %s: %s", mix.artist.c_str(), mix.title.c_str());
                }
            }
            return;
            
        case SDLK_v:
            // V: List favorite mixes
            {
                auto favoriteMixes = _mixManager->getFavoriteMixes();
                const char* color_cyan = "\033[36m";
                const char* color_yellow = "\033[33m";
                const char* color_green = "\033[32m";
                const char* color_magenta = "\033[35m";
                const char* color_red = "\033[31m";
                const char* color_reset = "\033[0m";
                
                ConsoleOutput::output("❤️  Favorite mixes (%zu):", favoriteMixes.size());
                for (const auto& mix : favoriteMixes) {
                    ConsoleOutput::output("  - %s: %s", mix.artist.c_str(), mix.title.c_str());
                }
            }
            return;
    }
}

void AutoVibezApp::displayMixStatus() {
    if (!_showMixStatus || !_mixManagerInitialized) return;
    
    if (_mixStatusDisplayTime > 0) {
        _mixStatusDisplayTime--;
        
        if (!_mixInfoDisplayed) {
            _mixUI->displayMixInfo(_currentMix);
            _mixInfoDisplayed = true;
        }
        
    } else {
        _showMixStatus = false;
    }
}

void AutoVibezApp::autoPlayOrDownload()
{
    if (!_mixManagerInitialized) {
        ConsoleOutput::output("❌ Mix manager not initialized for auto-play");
        ConsoleOutput::output("");
        return;
    }
    
    // Step 1: Check for existing downloaded mixes and play one, prioritizing preferred genre
    auto downloadedMixes = _mixManager->getDownloadedMixes();
    if (!downloadedMixes.empty()) {
        // First try to play a mix from preferred genre
        Mix randomMix = _mixManager->getRandomMixByGenre(_mixManager->getCurrentGenre());
        if (randomMix.id.empty()) {
            // If no mixes in preferred genre, try a favorite mix
            randomMix = _mixManager->getRandomFavoriteMix();
        }
        if (randomMix.id.empty()) {
            // If no favorites, fall back to any random mix
            randomMix = _mixManager->getRandomMix();
        }
        if (!randomMix.id.empty()) {
            if (_mixManager->playMix(randomMix)) {
                _currentMix = randomMix;
                _showMixStatus = true;
                _mixStatusDisplayTime = 300;
                _mixInfoDisplayed = false;
            } else {
                ConsoleOutput::output("❌ Play failed");
                ConsoleOutput::output("");
            }
        }
        
        // Start background download of remaining mixes
        startBackgroundDownloads();
        return;
    }
    
    // Step 2: No existing mixes, download and play one from preferred genre
    ConsoleOutput::output("📥 Downloading first mix...");
    ConsoleOutput::output("");
    
    // Try to get a mix from the preferred genre first
    Mix randomMix = _mixManager->getRandomAvailableMixByGenre(_mixManager->getCurrentGenre());
    if (randomMix.id.empty()) {
        // If no mixes in preferred genre, fall back to any random mix
        randomMix = _mixManager->getRandomAvailableMix();
    }
    if (randomMix.id.empty()) {
        ConsoleOutput::output("❌ No mixes available");
        ConsoleOutput::output("");
        return;
    }
    
    ConsoleOutput::output("🎵 Downloading: %s", randomMix.title.c_str());
    ConsoleOutput::output("");
    
    if (_mixManager->downloadAndAnalyzeMix(randomMix)) {
        // Get the updated mix with complete metadata from database
        Mix updatedMix = _mixManager->getMixById(randomMix.id);
        if (!updatedMix.id.empty()) {
            // Now play the analyzed mix
            if (_mixManager->playMix(updatedMix)) {
                _currentMix = updatedMix;
                _showMixStatus = true;
                _mixStatusDisplayTime = 300;
                _mixInfoDisplayed = false;
            } else {
                ConsoleOutput::output("❌ Play failed");
            }
        }
        
        // Start background download of remaining mixes
        startBackgroundDownloads();
    } else {
        ConsoleOutput::output("❌ Download failed");
    }
}

void AutoVibezApp::autoDownloadRandomMix()
{
    if (!_mixManagerInitialized) {
        ConsoleOutput::output("❌ Mix manager not initialized for auto-download");
        return;
    }
    
    // First try to get a random mix from available (not yet downloaded) mixes in preferred genre
    Mix randomMix = _mixManager->getRandomAvailableMixByGenre(_mixManager->getCurrentGenre());
    if (randomMix.id.empty()) {
        // If no available mixes in preferred genre, try any available mix
        randomMix = _mixManager->getRandomAvailableMix();
    }
    if (randomMix.id.empty()) {
        // If no available mixes, try database (already downloaded) - prioritize favorites
        randomMix = _mixManager->getRandomFavoriteMix();
    }
    if (randomMix.id.empty()) {
        // If no favorites, try database (already downloaded) in preferred genre
        randomMix = _mixManager->getRandomMixByGenre(_mixManager->getCurrentGenre());
    }
    if (randomMix.id.empty()) {
        // If no mixes in preferred genre, try any random mix from database
        randomMix = _mixManager->getRandomMix();
    }
    if (randomMix.id.empty()) {
        ConsoleOutput::output("❌ No mixes available for auto-download");
        return;
    }
    
    ConsoleOutput::output("🎵 Auto-downloading and analyzing: %s by %s", 
           randomMix.title.c_str(), randomMix.artist.c_str());
    
    // Use the new download-first approach
    if (_mixManager->downloadAndAnalyzeMix(randomMix)) {
        ConsoleOutput::output("✅ Auto-download and analysis successful");
        
        // Get the updated mix with complete metadata from database
        Mix updatedMix = _mixManager->getMixById(randomMix.id);
        if (!updatedMix.id.empty()) {
            // Now play the analyzed mix
            if (_mixManager->playMix(updatedMix)) {
                _currentMix = updatedMix; // Use the updated mix with metadata
                _showMixStatus = true;
                _mixStatusDisplayTime = 300;
                _mixInfoDisplayed = false;
                ConsoleOutput::output("✅ Auto-play successful");
            } else {
                ConsoleOutput::output("❌ Auto-play failed: %s", _mixManager->getLastError().c_str());
            }
        } else {
            ConsoleOutput::output("❌ Failed to retrieve updated mix from database");
        }
    } else {
        ConsoleOutput::output("❌ Auto-download and analysis failed: %s", _mixManager->getLastError().c_str());
    }
}

void AutoVibezApp::startBackgroundDownloads()
{
    if (!_mixManagerInitialized) {
        return;
    }
    
    // Get all available mixes that haven't been downloaded yet
    auto availableMixes = _mixManager->getAvailableMixes();
    
    // Count mixes that need to be downloaded by checking cache directly
    int pendingDownloads = 0;
    int pendingAnalysis = 0;
    std::vector<Mix> mixesToDownload;
    std::vector<Mix> mixesToAnalyze;
    
    for (const auto& mix : availableMixes) {
        // Check if the mix file actually exists in cache
        std::string cache_file = getCacheDirectory() + "/mix_cache/" + mix.id + ".mp3";
        if (!std::filesystem::exists(cache_file)) {
            pendingDownloads++;
            mixesToDownload.push_back(mix);
        } else {
            // Check if mix is in database
            Mix dbMix = _mixManager->getMixById(mix.id);
            if (dbMix.id.empty()) {
                pendingAnalysis++;
                mixesToAnalyze.push_back(mix);
            }
        }
    }
    
    if (pendingDownloads > 0) {
        ConsoleOutput::output("🔄 Downloading %d mixes in background...", pendingDownloads);
        
        // Download mixes in background (synchronously for now)
        for (const auto& mix : mixesToDownload) {
            // Download the mix (without analysis for now)
            if (_mixManager->downloadMixBackground(mix)) {
                // Silent success
            } else {
                ConsoleOutput::output("❌ Failed: %s", mix.title.c_str());
            }
        }
        
        ConsoleOutput::output("✅ Background downloads completed");
    }
    
    if (pendingAnalysis > 0) {
        ConsoleOutput::output("🔍 Analyzing %d existing mixes...", pendingAnalysis);
        
        // Analyze existing mixes and add to database
        for (const auto& mix : mixesToAnalyze) {
            if (_mixManager->downloadMixBackground(mix)) {
                // Silent success
            } else {
                ConsoleOutput::output("❌ Failed to analyze: %s", mix.title.c_str());
            }
        }
        
        ConsoleOutput::output("✅ Analysis completed");
    }
}

void AutoVibezApp::checkAndAutoPlayNext() {
    if (!_mixManagerInitialized || !_mixManager) {
        return;
    }
    
    // Check if music has stopped playing (not just paused)
    if (!_mixManager->isPlaying() && !_mixManager->isPaused()) {
        // Music has ended, play next random mix
        Mix nextMix = _mixManager->getSmartRandomMix(_currentMix.id, _mixManager->getCurrentGenre());
        if (!nextMix.id.empty()) {
            ConsoleOutput::output("🎵 Auto-playing next mix: %s by %s", nextMix.title.c_str(), nextMix.artist.c_str());
            if (_mixManager->downloadAndPlayMix(nextMix)) {
                _currentMix = nextMix;
                _showMixStatus = true;
                _mixStatusDisplayTime = 300;
                _mixInfoDisplayed = false;
            } else {
                ConsoleOutput::output("❌ Auto-play failed: %s", _mixManager->getLastError().c_str());
            }
        } else {
            ConsoleOutput::output("❌ No more mixes available for auto-play");
        }
    }
}

} // namespace Core
} // namespace AutoVibez