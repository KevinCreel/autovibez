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
    
}

AutoVibezApp::~AutoVibezApp()
{
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
        SDL_ShowCursor(true);
    }
    else
    {
        SDL_ShowCursor(false);
        SDL_SetWindowFullscreen(_sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        _isFullScreen = true;
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
        printf("⏮️  Previous preset: %s\n", preset_name.c_str());
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
        printf("⏭️  Next preset: %s\n", preset_name.c_str());
    }
}

void AutoVibezApp::keyHandler(SDL_Event* sdl_evt)
{
    // Initialize mix manager if not already done
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
                printf("🎵 Beat Sensitivity: %.1f\n", newSensitivity);
                printf("\n");
            }
            break;

        case SDLK_j:
            // J: Decrease beat sensitivity
            {
                float currentSensitivity = projectm_get_beat_sensitivity(_projectM);
                float newSensitivity = currentSensitivity - 0.1f;
                if (newSensitivity < 0.0f) newSensitivity = 0.0f;
                projectm_set_beat_sensitivity(_projectM, newSensitivity);
                printf("🎵 Beat Sensitivity: %.1f\n", newSensitivity);
                printf("\n");
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
            if (_simpleUI) {
                _simpleUI->toggle();
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
                    _mixInfoDisplayed = false; // Reset flag for new mix
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
                printf("⏮️  Previous preset: %s\n", preset_name.c_str());
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
                printf("⏭️  Next preset: %s\n", preset_name.c_str());
            }
            break;
            
        case SDLK_g:
            // G: Random mix in current genre
            if (_mixManagerInitialized) {
                // Check for modifier keys
                if (sdl_mod & KMOD_LSHIFT || sdl_mod & KMOD_RSHIFT) {
                    // Shift+G: Switch to random genre
                    std::string newGenre = _mixManager->getRandomGenre();
                    printf("🎼 Switched to genre: %s\n", newGenre.c_str());
                    printf("\n");
                    
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
                    printf("🎼 Available genres (%zu):\n", genres.size());
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
                        printf("  • %s\n", display_genre.c_str());
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
                    printf("🎼 Current genre: %s\n", display_current_genre.c_str());
                    printf("\n");
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
                            printf("❌ No other mixes found in genre: %s\n", _currentMix.genre.c_str());
                            printf("\n");
                        }
                    } else {
                        printf("❌ No mix currently playing or no genre available\n");
                        printf("\n");
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

void AutoVibezApp::resize(unsigned int width_, unsigned int height_)
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

void AutoVibezApp::pollEvent()
{
    SDL_Event evt;

    int mousex = 0;
    float mousexscale = 0;
    int mousey = 0;
    float mouseyscale = 0;
    int mousepressure = 0;
    while (SDL_PollEvent(&evt))
    {
        switch (evt.type)
        {
            case SDL_WINDOWEVENT:
                int h, w;
                SDL_GL_GetDrawableSize(_sdlWindow, &w, &h);
                switch (evt.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                        resize(w, h);
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        resize(w, h);
                        break;
                }
                break;
            case SDL_MOUSEWHEEL:
                scrollHandler(&evt);
                break;

            case SDL_KEYDOWN:
                keyHandler(&evt);
                break;
                
            case SDL_KEYUP:
                // Handle volume key release
                if (_volumeKeyPressed && (evt.key.keysym.sym == SDLK_UP || evt.key.keysym.sym == SDLK_DOWN)) {
                    printf("🔊 Volume: %d%%\n", _mixManager->getVolume());
                    printf("\n");
                    _volumeKeyPressed = false;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (evt.button.button == SDL_BUTTON_LEFT)
                {
                    // if it's the first mouse down event (since mouse up or since SDL was launched)
                    if (!mouseDown)
                    {
                        // Get mouse coorindates when you click.
                        SDL_GetMouseState(&mousex, &mousey);
                        // Scale those coordinates. libProjectM supports a scale of 0.1 instead of absolute pixel coordinates.
                        mousexscale = (mousex / (float) _width);
                        mouseyscale = ((_height - mousey) / (float) _height);
                        // Touch. By not supplying a touch type, we will default to random.
                        touch(mousexscale, mouseyscale, mousepressure);
                        mouseDown = true;
                    }
                }
                else if (evt.button.button == SDL_BUTTON_RIGHT)
                {
                    mouseDown = false;

                    // Keymod = Left or Right Gui or Left Ctrl. This is a shortcut to remove all waveforms.
                    if (keymod)
                    {
                        touchDestroyAll();
                        keymod = false;
                        break;
                    }

                    // Right Click
                    SDL_GetMouseState(&mousex, &mousey);

                    // Scale those coordinates. libProjectM supports a scale of 0.1 instead of absolute pixel coordinates.
                    mousexscale = (mousex / (float) _width);
                    mouseyscale = ((_height - mousey) / (float) _height);

                    // Destroy at the coordinates we clicked.
                    touchDestroy(mousexscale, mouseyscale);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                mouseDown = false;
                break;

            case SDL_QUIT:
                done = true;
                break;
        }
    }

    // Handle dragging your waveform when mouse is down.
    if (mouseDown)
    {
        // Get mouse coordinates when you click.
        SDL_GetMouseState(&mousex, &mousey);
        // Scale those coordinates. libProjectM supports a scale of 0.1 instead of absolute pixel coordinates.
        mousexscale = (mousex / (float) _width);
        mouseyscale = ((_height - mousey) / (float) _height);
        // Drag Touch.
        touchDrag(mousexscale, mouseyscale, mousepressure);
    }
}

// This touches the screen to generate a waveform at X / Y.
void AutoVibezApp::touch(float x, float y, int pressure, int touchtype)
{
#ifdef PROJECTM_TOUCH_ENABLED
    projectm_touch(_projectM, x, y, pressure, static_cast<projectm_touch_type>(touchtype));
#else
    (void)x; (void)y; (void)pressure; (void)touchtype; // Touch handling disabled
#endif
}

// This moves the X Y of your existing waveform that was generated by a touch (only if you held down your click and dragged your mouse around).
void AutoVibezApp::touchDrag(float x, float y, int pressure)
{
    projectm_touch_drag(_projectM, x, y, pressure);
}

// Remove waveform at X Y
void AutoVibezApp::touchDestroy(float x, float y)
{
    projectm_touch_destroy(_projectM, x, y);
}

// Remove all waveforms
void AutoVibezApp::touchDestroyAll()
{
    projectm_touch_destroy_all(_projectM);
}

void AutoVibezApp::renderFrame()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_opengl_render_frame(_projectM);

    // Render overlays
    renderHelpOverlay();
    renderFpsCounter();
    displayMixStatus();
    renderSimpleUI();

    SDL_GL_SwapWindow(_sdlWindow);
}

void AutoVibezApp::init(SDL_Window* window, const bool _renderToTexture)
{
    (void)_renderToTexture; // Parameter not used in current implementation
    _sdlWindow = window;
    projectm_set_window_size(_projectM, _width, _height);

#ifdef WASAPI_LOOPBACK
    wasapi = true;
#endif

    // Initialize simple UI
    _simpleUI = std::make_unique<SimpleUI>();
    if (_simpleUI) {
        _simpleUI->init(window, _openGlContext);
    }
}

void AutoVibezApp::initSimpleUI() {
    if (!_simpleUI) {
        _simpleUI = std::make_unique<SimpleUI>();
        if (_simpleUI) {
            _simpleUI->init(_sdlWindow, _openGlContext);
        }
    }
}

void AutoVibezApp::renderSimpleUI() {
    if (_simpleUI) {
        _simpleUI->render();
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
        
        printf("🎨 Preset: %s\n", preset_name.c_str());
    }
    
    // Reset the flag
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
        renderHelpOverlay();
    }
}



void AutoVibezApp::cycleAudioDevice()
{
    if (_numAudioDevices > 0) {
        _selectedAudioDeviceIndex = (_selectedAudioDeviceIndex + 1) % _numAudioDevices;
        
        // Get the device name for feedback
        const char* deviceName = SDL_GetAudioDeviceName(_selectedAudioDeviceIndex, SDL_TRUE);
        if (deviceName) {
            printf("🎚️  Switched to audio device: %s\n", deviceName);
        } else {
            printf("🎚️  Switched to audio device: %d\n", _selectedAudioDeviceIndex);
        }
        
        // Reopen audio with new device
        endAudioCapture();
        _curAudioDevice = _selectedAudioDeviceIndex;
        _selectedAudioDevice = _selectedAudioDeviceIndex;  // Set both variables
        initAudioInput();  // Use initAudioInput directly instead of openAudioInput
        beginAudioCapture();
    } else {
        printf("🎚️  No audio devices available\n");
    }
}

void AutoVibezApp::renderHelpOverlay()
{
    if (!_showHelp) return;
    static bool helpShown = false;
    if (!helpShown) {
        const char* color_bright_green = "\033[92m";
        const char* color_bright_yellow = "\033[93m";
        const char* color_bright_blue = "\033[94m";
        const char* color_bright_magenta = "\033[95m";
        const char* color_bright_cyan = "\033[96m";
        const char* color_bright_red = "\033[91m";
        const char* color_reset = "\033[0m";
        
        printf("\n");
        printf("%s✨🎵 AutoVibez Controls 🎵✨%s\n", color_bright_cyan, color_reset);
        printf("\n");
        printf("%s🎧 Mix Management:%s\n", color_bright_magenta, color_reset);
        printf("%sN%s     - Play next mix\n", color_bright_yellow, color_reset);
        printf("%sF%s     - Toggle favorite\n", color_bright_yellow, color_reset);
        printf("%sV%s     - List favorite mixes\n", color_bright_yellow, color_reset);
        printf("%sL%s     - List available mixes\n", color_bright_yellow, color_reset);
        printf("%sG%s     - Play random mix in current genre\n", color_bright_yellow, color_reset);
        printf("%sShift+G%s - Switch to random genre\n", color_bright_yellow, color_reset);
        printf("%sCtrl+G%s  - Show available genres\n", color_bright_yellow, color_reset);
        printf("%sSPACE%s  - Load random mix\n", color_bright_yellow, color_reset);
        printf("\n");
        printf("%s🎚️  Audio Controls:%s\n", color_bright_blue, color_reset);
        printf("%sP%s     - Pause/Resume playback\n", color_bright_green, color_reset);
        printf("%s↑/↓%s   - Volume up/down\n", color_bright_green, color_reset);
        printf("%sTab%s   - Cycle through audio devices\n", color_bright_green, color_reset);
        printf("\n");
        printf("%s🌈 Visualizer Controls:%s\n", color_bright_magenta, color_reset);
        printf("%sH%s     - Toggle this help overlay\n", color_bright_cyan, color_reset);
        printf("%sF11%s   - Toggle fullscreen mode\n", color_bright_cyan, color_reset);
        printf("%sR%s     - Load random preset\n", color_bright_cyan, color_reset);
        printf("%s[ / ]%s  - Previous/Next preset\n", color_bright_cyan, color_reset);
        printf("%sB / J%s  - Increase/Decrease beat sensitivity\n", color_bright_cyan, color_reset);
        printf("%sMouse Wheel%s - Next/Prev preset\n", color_bright_cyan, color_reset);
        printf("\n");
        printf("%s⚙️  Application:%s\n", color_bright_red, color_reset);
        printf("%sCtrl+Q%s - Quit application\n", color_bright_red, color_reset);
        printf("\n");
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
        printf("FPS: %.1f\n", current_fps);
        frameCount = 0;
        lastTime = currentTime;
    }
}

// Get XDG data directory for autovibez (cross-platform)
std::string AutoVibezApp::getDataDirectory() {
    std::string data_dir;
    
#ifdef _WIN32
    // Windows: Use %APPDATA%/autovibez
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        data_dir = std::string(appdata) + "/autovibez";
    } else {
        data_dir = "data"; // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Application Support/autovibez
    const char* home = std::getenv("HOME");
    if (home) {
        data_dir = std::string(home) + "/Library/Application Support/autovibez";
    } else {
        data_dir = "data"; // Fallback
    }
#else
    // Linux/Unix: Use XDG Base Directory Specification
    const char* xdg_data_home = std::getenv("XDG_DATA_HOME");
    if (xdg_data_home && strlen(xdg_data_home) > 0) {
        data_dir = std::string(xdg_data_home) + "/autovibez";
    } else {
        // Fallback to default XDG location
        const char* home = std::getenv("HOME");
        if (home) {
            data_dir = std::string(home) + "/.local/share/autovibez";
        } else {
            data_dir = "data"; // Last resort fallback
        }
    }
#endif
    
    // Create directory if it doesn't exist
    if (!std::filesystem::exists(data_dir)) {
        std::filesystem::create_directories(data_dir);
    }
    
    return data_dir;
}

// Get XDG config directory for autovibez (cross-platform)
std::string AutoVibezApp::getConfigDirectory() {
    std::string config_dir;
    
#ifdef _WIN32
    // Windows: Use %APPDATA%/autovibez/config
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        config_dir = std::string(appdata) + "/autovibez/config";
    } else {
        config_dir = "config"; // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Application Support/autovibez/config
    const char* home = std::getenv("HOME");
    if (home) {
        config_dir = std::string(home) + "/Library/Application Support/autovibez/config";
    } else {
        config_dir = "config"; // Fallback
    }
#else
    // Linux/Unix: Use XDG Base Directory Specification
    const char* xdg_config_home = std::getenv("XDG_CONFIG_HOME");
    if (xdg_config_home && strlen(xdg_config_home) > 0) {
        config_dir = std::string(xdg_config_home) + "/autovibez";
    } else {
        // Fallback to default XDG location
        const char* home = std::getenv("HOME");
        if (home) {
            config_dir = std::string(home) + "/.config/autovibez";
        } else {
            config_dir = "config"; // Last resort fallback
        }
    }
#endif
    
    // Create directory if it doesn't exist
    if (!std::filesystem::exists(config_dir)) {
        std::filesystem::create_directories(config_dir);
    }
    
    return config_dir;
}

// Get XDG cache directory for autovibez (cross-platform)
std::string AutoVibezApp::getCacheDirectory() {
    std::string cache_dir;
    
#ifdef _WIN32
    // Windows: Use %LOCALAPPDATA%/autovibez/cache
    const char* localappdata = std::getenv("LOCALAPPDATA");
    if (localappdata) {
        cache_dir = std::string(localappdata) + "/autovibez/cache";
    } else {
        cache_dir = "cache"; // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Caches/autovibez
    const char* home = std::getenv("HOME");
    if (home) {
        cache_dir = std::string(home) + "/Library/Caches/autovibez";
    } else {
        cache_dir = "cache"; // Fallback
    }
#else
    // Linux/Unix: Use XDG Base Directory Specification
    const char* xdg_cache_home = std::getenv("XDG_CACHE_HOME");
    if (xdg_cache_home && strlen(xdg_cache_home) > 0) {
        cache_dir = std::string(xdg_cache_home) + "/autovibez";
    } else {
        // Fallback to default XDG location
        const char* home = std::getenv("HOME");
        if (home) {
            cache_dir = std::string(home) + "/.cache/autovibez";
        } else {
            cache_dir = "cache"; // Last resort fallback
        }
    }
#endif
    
    // Create directory if it doesn't exist
    if (!std::filesystem::exists(cache_dir)) {
        std::filesystem::create_directories(cache_dir);
    }
    
    return cache_dir;
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
    std::string assets_dir;
    
#ifdef _WIN32
    // Windows: Use %APPDATA%/autovibez/assets
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        assets_dir = std::string(appdata) + "/autovibez/assets";
    } else {
        assets_dir = "assets"; // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Application Support/autovibez/assets
    const char* home = std::getenv("HOME");
    if (home) {
        assets_dir = std::string(home) + "/Library/Application Support/autovibez/assets";
    } else {
        assets_dir = "assets"; // Fallback
    }
#else
    // Linux/Unix: Use XDG Base Directory Specification
    const char* xdg_data_home = std::getenv("XDG_DATA_HOME");
    if (xdg_data_home && strlen(xdg_data_home) > 0) {
        assets_dir = std::string(xdg_data_home) + "/autovibez/assets";
    } else {
        // Fallback to default XDG location
        const char* home = std::getenv("HOME");
        if (home) {
            assets_dir = std::string(home) + "/.local/share/autovibez/assets";
        } else {
            assets_dir = "assets"; // Last resort fallback
        }
    }
#endif
    
    // Create directory if it doesn't exist
    if (!std::filesystem::exists(assets_dir)) {
        std::filesystem::create_directories(assets_dir);
    }
    
    return assets_dir;
}

// Mix management methods
void AutoVibezApp::initMixManager()
{
    if (_mixManagerInitialized) return;
    
    // Initialize mix manager with XDG data directory
    std::string data_dir = getDataDirectory();
    std::string db_path = getStateDirectory() + "/autovibez_mixes.db";
    std::string cache_dir = getCacheDirectory() + "/mix_cache";
    
    _mixManager = std::make_unique<MixManager>(db_path, cache_dir);
    _mixUI = std::make_unique<MixDisplay>();
    
    if (!_mixManager->initialize()) {
        printf("❌ Failed to initialize mix manager: %s\n", _mixManager->getLastError().c_str());
        return;
    }
    
    // Load configuration
    std::string configFilePath = findConfigFile();
    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);
        
        // Set initial genre from config
        std::string preferred_genre = config.getPreferredGenre();
        if (!preferred_genre.empty()) {
            printf("⚙️  Setting preferred genre from config: '%s'\n\n", preferred_genre.c_str());
        } else {
            printf("⚙️  No preferred genre set in config - will use random genres\n\n");
        }
        _mixManager->setCurrentGenre(preferred_genre);
        
        // Show current audio device
        int audioDeviceIndex = config.getAudioDeviceIndex();
        const char* deviceName = SDL_GetAudioDeviceName(audioDeviceIndex, SDL_TRUE);
        if (deviceName) {
            printf("🎚️  Using audio device: %s\n\n", deviceName);
        } else {
            printf("🎚️  Using audio device: %d\n\n", audioDeviceIndex);
        }
    }
    
    // Load mix metadata from YAML
    std::string yaml_url;
    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);
        yaml_url = config.getMixesUrl();
    }
    
    if (yaml_url.empty()) {
        printf("⚠️  No mixes_url configured, skipping mix metadata loading\n");
        return;
    }
    
    if (!_mixManager->loadMixMetadata(yaml_url)) {
        printf("❌ Failed to load mix metadata: %s\n", _mixManager->getLastError().c_str());
        return;
    }
    
    // Check for new mixes from remote YAML in background (non-blocking)
    // This will download new mixes but not block startup
    std::thread([this, yaml_url]() {
        _mixManager->checkForNewMixes(yaml_url);
    }).detach();
    
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
                    printf("🎨 Loaded random preset: %s\n", preset_name.c_str());
                }
            }
            return; // Return to prevent double handling
            
        case SDLK_n:
            // N: Next mix
            {
                Mix nextMix = _mixManager->getNextMix(_currentMix.id);
                if (!nextMix.id.empty()) {
                    _mixManager->downloadAndPlayMix(nextMix);
                    _currentMix = nextMix;
                    _showMixStatus = true;
                    _mixStatusDisplayTime = 300;
                    _mixInfoDisplayed = false; // Reset flag for new mix
                }
            }
            return; // Return to prevent double handling
            
        case SDLK_p:
            // P: Pause/Resume
            _mixManager->togglePause();
            return; // Return to prevent double handling
            
        case SDLK_UP:
            // ↑: Volume up
            {
                int currentVolume = _mixManager->getVolume();
                _mixManager->setVolume(currentVolume + 10, true); // Always suppress output
                _volumeKeyPressed = true;
            }
            return; // Return to prevent double handling
            
        case SDLK_DOWN:
            // ↓: Volume down
            {
                int currentVolume = _mixManager->getVolume();
                _mixManager->setVolume(currentVolume - 10, true); // Always suppress output
                _volumeKeyPressed = true;
            }
            return; // Return to prevent double handling
            
        case SDLK_1:
        case SDLK_2:
        case SDLK_3:
        case SDLK_4:
        case SDLK_5:

            
        case SDLK_f:
            // F: Toggle favorite
            if (!_currentMix.id.empty()) {
                _mixManager->toggleFavorite(_currentMix.id);
                printf("❤️  Toggled favorite for %s\n", _currentMix.title.c_str());
                printf("\n");
            }
            return; // Return to prevent double handling
            
        case SDLK_l:
            // L: List downloaded mixes
            {
                auto downloadedMixes = _mixManager->getDownloadedMixes();
                const char* color_cyan = "\033[36m";
                const char* color_yellow = "\033[33m";
                const char* color_green = "\033[32m";
                const char* color_magenta = "\033[35m";
                const char* color_reset = "\033[0m";
                
                printf("%s📋 %sDownloaded mixes (%zu):%s\n", color_cyan, color_green, downloadedMixes.size(), color_reset);
                for (const auto& mix : downloadedMixes) {
                    printf("  %s- %s%s%s: %s%s%s\n", 
                           color_yellow, color_magenta, mix.artist.c_str(), color_reset,
                           color_cyan, mix.title.c_str(), color_reset);
                }
            }
            return; // Return to prevent double handling
            
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
                
                printf("%s❤️  %sFavorite mixes (%zu):%s\n", color_red, color_green, favoriteMixes.size(), color_reset);
                for (const auto& mix : favoriteMixes) {
                    printf("  %s- %s%s%s: %s%s%s\n", 
                           color_yellow, color_magenta, mix.artist.c_str(), color_reset,
                           color_cyan, mix.title.c_str(), color_reset);
                }
            }
            return; // Return to prevent double handling
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
        printf("❌ Mix manager not initialized for auto-play\n");
        printf("\n");
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
                printf("❌ Play failed\n");
                printf("\n");
            }
        }
        
        // Start background download of remaining mixes
        startBackgroundDownloads();
        return;
    }
    
    // Step 2: No existing mixes, download and play one from preferred genre
    printf("📥 Downloading first mix...\n");
    printf("\n");
    
    // Try to get a mix from the preferred genre first
    Mix randomMix = _mixManager->getRandomAvailableMixByGenre(_mixManager->getCurrentGenre());
    if (randomMix.id.empty()) {
        // If no mixes in preferred genre, fall back to any random mix
        randomMix = _mixManager->getRandomAvailableMix();
    }
    if (randomMix.id.empty()) {
        printf("❌ No mixes available\n");
        printf("\n");
        return;
    }
    
    printf("🎵 Downloading: %s\n", randomMix.title.c_str());
    printf("\n");
    
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
                printf("❌ Play failed\n");
            }
        }
        
        // Start background download of remaining mixes
        startBackgroundDownloads();
    } else {
        printf("❌ Download failed\n");
    }
}

void AutoVibezApp::autoDownloadRandomMix()
{
    if (!_mixManagerInitialized) {
        printf("❌ Mix manager not initialized for auto-download\n");
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
        printf("❌ No mixes available for auto-download\n");
        return;
    }
    
    printf("🎵 Auto-downloading and analyzing: %s by %s\n", 
           randomMix.title.c_str(), randomMix.artist.c_str());
    
    // Use the new download-first approach
    if (_mixManager->downloadAndAnalyzeMix(randomMix)) {
        printf("✅ Auto-download and analysis successful\n");
        
        // Get the updated mix with complete metadata from database
        Mix updatedMix = _mixManager->getMixById(randomMix.id);
        if (!updatedMix.id.empty()) {
            // Now play the analyzed mix
            if (_mixManager->playMix(updatedMix)) {
                _currentMix = updatedMix; // Use the updated mix with metadata
                _showMixStatus = true;
                _mixStatusDisplayTime = 300;
                _mixInfoDisplayed = false; // Reset flag for new mix
                printf("✅ Auto-play successful\n");
            } else {
                printf("❌ Auto-play failed: %s\n", _mixManager->getLastError().c_str());
            }
        } else {
            printf("❌ Failed to retrieve updated mix from database\n");
        }
    } else {
        printf("❌ Auto-download and analysis failed: %s\n", _mixManager->getLastError().c_str());
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
        printf("🔄 Downloading %d mixes in background...\n", pendingDownloads);
        
        // Download mixes in background (synchronously for now)
        for (const auto& mix : mixesToDownload) {
            // Download the mix (without analysis for now)
            if (_mixManager->downloadMixBackground(mix)) {
                // Silent success
            } else {
                printf("❌ Failed: %s\n", mix.title.c_str());
            }
        }
        
        printf("✅ Background downloads completed\n");
    }
    
    if (pendingAnalysis > 0) {
        printf("🔍 Analyzing %d existing mixes...\n", pendingAnalysis);
        
        // Analyze existing mixes and add to database
        for (const auto& mix : mixesToAnalyze) {
            if (_mixManager->downloadMixBackground(mix)) {
                // Silent success
            } else {
                printf("❌ Failed to analyze: %s\n", mix.title.c_str());
            }
        }
        
        printf("✅ Analysis completed\n");
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
            printf("🎵 Auto-playing next mix: %s by %s\n", nextMix.title.c_str(), nextMix.artist.c_str());
            if (_mixManager->downloadAndPlayMix(nextMix)) {
                _currentMix = nextMix;
                _showMixStatus = true;
                _mixStatusDisplayTime = 300;
                _mixInfoDisplayed = false; // Reset flag for new mix
            } else {
                printf("❌ Auto-play failed: %s\n", _mixManager->getLastError().c_str());
            }
        } else {
            printf("❌ No more mixes available for auto-play\n");
        }
    }
}



