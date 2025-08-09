#include "autovibez_app.hpp"

#include <backends/imgui_impl_sdl2.h>

#include <filesystem>
#include <thread>
#include <vector>

#include "config_manager.hpp"
#include "console_output.hpp"
#include "constants.hpp"
#include "mix_downloader.hpp"
#include "mix_manager.hpp"
#include "mix_metadata.hpp"
#include "overlay_messages.hpp"
#include "path_manager.hpp"
#include "setup.hpp"
#include "utils/logger.hpp"
#if defined _MSC_VER
#include <direct.h>
#else
#include <sys/stat.h>
#endif

using AutoVibez::Data::MixManager;

using AutoVibez::Data::ConfigFile;
using AutoVibez::Data::Mix;
using AutoVibez::UI::HelpOverlay;

namespace AutoVibez::Core {

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

    // Initialize KeyBindingManager
    _keyBindingManager = std::make_unique<KeyBindingManager>();

    // Initialize System Volume Controller
    _systemVolumeController = AutoVibez::Utils::SystemVolumeControllerFactory::create();

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

void AutoVibezApp::resizeWindow(unsigned int width_, unsigned int height_) {
    _width = width_;
    _height = height_;

    // Hide cursor if window size equals desktop size
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        SDL_ShowCursor(_isFullScreen ? SDL_DISABLE : SDL_ENABLE);
    }

    projectm_set_window_size(_projectM, _width, _height);

    // Update message overlay window size
    if (_messageOverlay) {
        _messageOverlay->setWindowSize(_width, _height);
    }
}

void AutoVibezApp::pollEvents() {
    SDL_Event evt;
    while (SDL_PollEvent(&evt)) {
        // Pass events to ImGui when help overlay is visible and ImGui is ready
        if (_helpOverlay && _helpOverlay->isVisible() && _helpOverlay->isImGuiReady()) {
            ImGui_ImplSDL2_ProcessEvent(&evt);
            // For mouse wheel events, let ImGui handle them exclusively when overlay is visible
            if (evt.type == SDL_MOUSEWHEEL) {
                continue;
            }
        }

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
                handleQuitEvent();
                break;
            default:
                break;
        }
    }
}

void AutoVibezApp::handleWindowEvent(const SDL_Event& evt) {
    int w, h;
    SDL_GL_GetDrawableSize(_sdlWindow, &w, &h);
    switch (evt.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            // Ensure positive values before casting to unsigned
            if (w > 0 && h > 0) {
                resizeWindow(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
            }
            break;
        default:
            break;
    }
}

void AutoVibezApp::handleKeyDownEvent(const SDL_Event& evt) {
    // Ensure mix manager is initialized before handling any keys
    if (!_mixManagerInitialized) {
        initMixManager();
    }

    // Try KeyBindingManager first - this should handle ALL keys
    if (_keyBindingManager && _keyBindingManager->handleKey(const_cast<SDL_Event*>(&evt))) {
        return;  // Key was handled successfully
    }

    // No fallback needed if KeyBindingManager is complete
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

void AutoVibezApp::handleQuitEvent() {
    done = true;
}

void AutoVibezApp::renderFrame() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_opengl_render_frame(_projectM);

    // Render overlays
    renderHelpOverlay();
    renderMessageOverlay();

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

    // Initialize message overlay
    initMessageOverlay();

    // Initialize key binding manager actions
    initKeyBindingManager();
}

void AutoVibezApp::initHelpOverlay() {
    if (!_helpOverlay) {
        _helpOverlay = std::make_unique<HelpOverlay>();
        if (_helpOverlay) {
            _helpOverlay->init(_sdlWindow, _openGlContext);
        }
    }
}

void AutoVibezApp::initMessageOverlay() {
    if (!_messageOverlay) {
        _messageOverlay = std::make_unique<AutoVibez::UI::MessageOverlayWrapper>();
        if (_messageOverlay) {
            _messageOverlay->init(_sdlWindow, _openGlContext);

            // Connect message overlay to mix manager
            if (_mixManager) {
                _mixManager->setMessageOverlay(_messageOverlay.get());
            }

            // Connect help overlay to message overlay for coordination
            if (_helpOverlay) {
                _helpOverlay->setMessageOverlay(_messageOverlay->getMessageOverlay());
            }
        }
    }
}

void AutoVibezApp::initKeyBindingManager() {
    if (!_keyBindingManager) {
        return;
    }

    // Register action callbacks for mix management
    _keyBindingManager->registerAction(KeyAction::PREVIOUS_MIX, [this]() {
        if (!_mixManagerInitialized)
            return;
        Mix prevMix = _mixManager->getPreviousMix(_currentMix.id);
        if (!prevMix.id.empty()) {
            AutoVibez::Utils::ConsoleOutput::mixInfo(prevMix.artist, prevMix.title, prevMix.genre);
            _mixManager->downloadAndPlayMix(prevMix);
            _currentMix = prevMix;
        }
    });

    _keyBindingManager->registerAction(KeyAction::NEXT_MIX, [this]() {
        if (!_mixManagerInitialized)
            return;
        Mix nextMix = _mixManager->getNextMix(_currentMix.id);
        if (!nextMix.id.empty()) {
            AutoVibez::Utils::ConsoleOutput::mixInfo(nextMix.artist, nextMix.title, nextMix.genre);
            _mixManager->downloadAndPlayMix(nextMix);
            _currentMix = nextMix;
        }
    });

    _keyBindingManager->registerAction(KeyAction::TOGGLE_FAVORITE, [this]() {
        if (!_currentMix.id.empty()) {
            bool wasFavorite = _currentMix.is_favorite;
            _mixManager->toggleFavorite(_currentMix.id);
            _currentMix.is_favorite = !wasFavorite;  // Update local state

            if (_currentMix.is_favorite) {
                AutoVibez::Utils::ConsoleOutput::success("Added to favorites: " + _currentMix.title);
            } else {
                AutoVibez::Utils::ConsoleOutput::info("Removed from favorites: " + _currentMix.title);
            }
        }
    });

    _keyBindingManager->registerAction(KeyAction::SHOW_MIX_INFO, [this]() {
        if (!_currentMix.id.empty() && _messageOverlay) {
            auto config =
                AutoVibez::Utils::OverlayMessages::createMessage("mix_info", _currentMix.artist, _currentMix.title);
            _messageOverlay->showMessage(config);
        }
    });

    _keyBindingManager->registerAction(KeyAction::SOFT_DELETE_MIX, [this]() {
        if (!_currentMix.id.empty()) {
            _mixManager->softDeleteMix(_currentMix.id);
            // Skip to next mix since current one is now deleted
            Mix nextMix = _mixManager->getNextMix(_currentMix.id);
            if (!nextMix.id.empty()) {
                _mixManager->downloadAndPlayMix(nextMix);
                _currentMix = nextMix;
            }
        }
    });

    _keyBindingManager->registerAction(KeyAction::TOGGLE_MIX_TABLE_FILTER, [this]() {
        if (_helpOverlay && _helpOverlay->isVisible()) {
            _helpOverlay->toggleMixTableFilter();
        }
    });

    _keyBindingManager->registerAction(KeyAction::RANDOM_MIX_CURRENT_GENRE, [this]() {
        if (!_mixManagerInitialized)
            return;
        if (!_currentMix.id.empty() && !_currentMix.genre.empty()) {
            Mix genreMix = _mixManager->getRandomMixByGenre(_currentMix.genre, _currentMix.id);
            if (!genreMix.id.empty()) {
                AutoVibez::Utils::ConsoleOutput::info("Playing random " + _currentMix.genre + " mix");
                AutoVibez::Utils::ConsoleOutput::mixInfo(genreMix.artist, genreMix.title, genreMix.genre);
                if (_mixManager->downloadAndPlayMix(genreMix)) {
                    _currentMix = genreMix;
                    if (_messageOverlay) {
                        auto config = AutoVibez::Utils::OverlayMessages::createMessage("mix_info", _currentMix.artist,
                                                                                       _currentMix.title);
                        _messageOverlay->showMessage(config);
                    }
                } else {
                    if (_messageOverlay) {
                        _messageOverlay->showMessage("Failed to load new mix");
                    }
                }
            }
        }
    });

    _keyBindingManager->registerAction(KeyAction::RANDOM_GENRE_AND_MIX, [this]() {
        if (!_mixManagerInitialized)
            return;
        std::string newGenre = _mixManager->getRandomGenre();
        AutoVibez::Utils::ConsoleOutput::info("Switched to genre: " + newGenre);

        Mix genreMix = _mixManager->getRandomMixByGenre(newGenre, _currentMix.id);
        if (!genreMix.id.empty()) {
            AutoVibez::Utils::ConsoleOutput::mixInfo(genreMix.artist, genreMix.title, genreMix.genre);
            if (_mixManager->downloadAndPlayMix(genreMix)) {
                _currentMix = genreMix;
                if (_messageOverlay) {
                    auto config = AutoVibez::Utils::OverlayMessages::createMessage("mix_info", _currentMix.artist,
                                                                                   _currentMix.title);
                    _messageOverlay->showMessage(config);
                }
            } else {
                if (_messageOverlay) {
                    _messageOverlay->showMessage("Failed to load mix from " + newGenre + " genre");
                }
            }
        }
    });

    _keyBindingManager->registerAction(KeyAction::PAUSE_RESUME_MIX, [this]() {
        if (_mixManagerInitialized) {
            _mixManager->togglePause();
        }
    });

    // Register action callbacks for visualizer controls
    _keyBindingManager->registerAction(KeyAction::TOGGLE_HELP_OVERLAY, [this]() {
        if (_helpOverlay) {
            _helpOverlay->toggle();
        }
    });

    _keyBindingManager->registerAction(KeyAction::TOGGLE_FULLSCREEN, [this]() { toggleFullScreen(); });

    _keyBindingManager->registerAction(KeyAction::RANDOM_PRESET, [this]() {
        if (_presetManager) {
            _presetManager->randomPreset();
            std::string preset_name = getActivePresetName();
            size_t last_slash = preset_name.find_last_of('/');
            if (last_slash != std::string::npos) {
                preset_name = preset_name.substr(last_slash + 1);
            }
            AutoVibez::Utils::ConsoleOutput::presetChange(preset_name);
        }
    });

    _keyBindingManager->registerAction(KeyAction::PREVIOUS_PRESET_BRACKET, [this]() {
        _manualPresetChange = true;
        projectm_playlist_play_previous(_playlist, true);
        std::string preset_name = getActivePresetName();
        size_t last_slash = preset_name.find_last_of('/');
        if (last_slash != std::string::npos) {
            preset_name = preset_name.substr(last_slash + 1);
        }
        AutoVibez::Utils::ConsoleOutput::presetChange(preset_name);
    });

    _keyBindingManager->registerAction(KeyAction::NEXT_PRESET_BRACKET, [this]() {
        _manualPresetChange = true;
        projectm_playlist_play_next(_playlist, true);
        std::string preset_name = getActivePresetName();
        size_t last_slash = preset_name.find_last_of('/');
        if (last_slash != std::string::npos) {
            preset_name = preset_name.substr(last_slash + 1);
        }
        AutoVibez::Utils::ConsoleOutput::presetChange(preset_name);
    });

    _keyBindingManager->registerAction(KeyAction::INCREASE_BEAT_SENSITIVITY, [this]() {
        float newSensitivity = getBeatSensitivity() + 0.1f;
        if (newSensitivity > 1.0f)
            newSensitivity = 1.0f;
        setBeatSensitivity(newSensitivity);
        AutoVibez::Utils::ConsoleOutput::info(
            "Beat sensitivity: " + std::to_string(static_cast<int>(newSensitivity * 100)) + "%");
    });

    _keyBindingManager->registerAction(KeyAction::DECREASE_BEAT_SENSITIVITY, [this]() {
        float newSensitivity = getBeatSensitivity() - 0.1f;
        if (newSensitivity < 0.0f)
            newSensitivity = 0.0f;
        setBeatSensitivity(newSensitivity);
        AutoVibez::Utils::ConsoleOutput::info(
            "Beat sensitivity: " + std::to_string(static_cast<int>(newSensitivity * 100)) + "%");
    });

    // Register action callbacks for application
    _keyBindingManager->registerAction(KeyAction::QUIT_WITH_MODIFIER, [this]() { done = true; });

    // Register action callbacks for audio controls
    _keyBindingManager->registerAction(KeyAction::TOGGLE_MUTE, [this]() {
        if (!_mixManagerInitialized)
            return;
        int currentVolume = _mixManager->getVolume();
        if (currentVolume > 0) {
            _previousVolume = currentVolume;
            _mixManager->setVolume(0, true);
            AutoVibez::Utils::ConsoleOutput::info("Audio muted");
        } else {
            _mixManager->setVolume(_previousVolume, true);
            AutoVibez::Utils::ConsoleOutput::info("Audio unmuted");
        }
    });

    _keyBindingManager->registerAction(KeyAction::VOLUME_UP, [this]() {
        if (_systemVolumeController && _systemVolumeController->isAvailable()) {
            int oldVolume = _systemVolumeController->getCurrentVolume();
            _systemVolumeController->increaseVolume(Constants::VOLUME_STEP_SIZE);
            int newVolume = _systemVolumeController->getCurrentVolume();
            AutoVibez::Utils::ConsoleOutput::volumeChange(oldVolume, newVolume);
        }
        _volumeKeyPressed = true;
    });

    _keyBindingManager->registerAction(KeyAction::VOLUME_DOWN, [this]() {
        if (_systemVolumeController && _systemVolumeController->isAvailable()) {
            int oldVolume = _systemVolumeController->getCurrentVolume();
            _systemVolumeController->decreaseVolume(Constants::VOLUME_STEP_SIZE);
            int newVolume = _systemVolumeController->getCurrentVolume();
            AutoVibez::Utils::ConsoleOutput::volumeChange(oldVolume, newVolume);
        }
        _volumeKeyPressed = true;
    });

    _keyBindingManager->registerAction(KeyAction::CYCLE_AUDIO_DEVICE, [this]() { cycleAudioDevice(); });
}

void AutoVibezApp::renderHelpOverlay() {
    if (_helpOverlay) {
        // Update help overlay with current information
        updateHelpOverlayInfo();
        _helpOverlay->render();
    }
}

void AutoVibezApp::renderMessageOverlay() {
    if (_messageOverlay) {
        _messageOverlay->render();
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
    if (_systemVolumeController && _systemVolumeController->isAvailable()) {
        int systemVolume = _systemVolumeController->getCurrentVolume();
        if (systemVolume >= 0) {
            _helpOverlay->setVolumeLevel(systemVolume);
        }
    } else if (_mixManager) {
        // Fallback to mix volume if system volume not available
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

        // Add console output for automatic preset changes (only if not manual)
        if (!app->_manualPresetChange) {
            std::string displayName = presetNameString;
            size_t lastSlash = displayName.find_last_of('/');
            if (lastSlash != std::string::npos) {
                displayName = displayName.substr(lastSlash + 1);
            }
            AutoVibez::Utils::ConsoleOutput::presetChange(displayName);
        }

        // Reset the manual preset change flag
        app->_manualPresetChange = false;

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
        ::AutoVibez::Utils::Logger logger;
        logger.logWarning("No audio capture devices available");
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

    // Connect message overlay to mix manager
    if (_messageOverlay) {
        _mixManager->setMessageOverlay(_messageOverlay.get());
    }

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
            AutoVibez::Utils::ConsoleOutput::info("Auto-playing next mix...");
            AutoVibez::Utils::ConsoleOutput::mixInfo(nextMix.artist, nextMix.title, nextMix.genre);
            if (_mixManager->downloadAndPlayMix(nextMix)) {
                _currentMix = nextMix;
            } else {
                // Auto-play failed, try another mix
                AutoVibez::Utils::ConsoleOutput::warning("Failed to play mix, trying another...");
                nextMix = _mixManager->getSmartRandomMix(nextMix.id, _mixManager->getCurrentGenre());
                if (!nextMix.id.empty()) {
                    AutoVibez::Utils::ConsoleOutput::mixInfo(nextMix.artist, nextMix.title, nextMix.genre);
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

}  // namespace AutoVibez::Core