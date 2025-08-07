#include "setup.hpp"

#include "autovibez_app.hpp"
#include "config_manager.hpp"
#include "constants.hpp"
#include "path_manager.hpp"
using AutoVibez::Core::AutoVibezApp;
#include <SDL2/SDL.h>
#include <SDL2/SDL_hints.h>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>

using AutoVibez::Audio::configureLoopback;
using AutoVibez::Audio::initLoopback;
using AutoVibez::Data::ConfigFile;
using AutoVibez::Data::MixManager;

std::string expandTilde(const std::string& path) {
    if (path.empty() || path[0] != '~') {
        return path;
    }

#ifdef _WIN32
    // Windows: Use %USERPROFILE% or %HOME%
    const char* userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        return std::string(userprofile) + path.substr(1);
    }
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + path.substr(1);
    }
#else
    // Unix-like systems: Use $HOME
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + path.substr(1);
    }
#endif

    return path;
}

#if OGL_DEBUG
void debugGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
             const void* userParam) {
    // Debug output disabled
}
#endif

std::string getConfigFilePath(const std::string& datadir_path) {
    std::string config_path = datadir_path + "/config.inp";

    std::ifstream f_config(config_path);
    if (f_config.good()) {
        return config_path;
    }

    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Config file not found: %s\n", config_path.c_str());
    return "";
}

// Get XDG config directory for autovibez (cross-platform)
std::string getConfigDirectory() {
    std::string config_dir;

#ifdef _WIN32
    // Windows: Use %APPDATA%/autovibez/config
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        config_dir = std::string(appdata) + "/autovibez/config";
    } else {
        config_dir = "config";  // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Application Support/autovibez/config
    const char* home = std::getenv("HOME");
    if (home) {
        config_dir = std::string(home) + "/Library/Application Support/autovibez/config";
    } else {
        config_dir = "config";  // Fallback
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
            config_dir = "config";  // Last resort fallback
        }
    }
#endif

    if (!std::filesystem::exists(config_dir)) {
        std::filesystem::create_directories(config_dir);
    }

    return config_dir;
}

// Find config file
std::string findConfigFile() {
    const char* config_env = std::getenv("AUTOVIBEZ_CONFIG");
    if (config_env && std::filesystem::exists(config_env)) {
        return config_env;
    }

    std::string xdg_config_path = getConfigDirectory() + "/config.inp";
    if (std::filesystem::exists(xdg_config_path)) {
        return xdg_config_path;
    }

    // Fallback to local config directory
    if (std::filesystem::exists("config/config.inp")) {
        return "config/config.inp";
    }

    return "";
}

// Get XDG assets directory for autovibez (cross-platform)
std::string getAssetsDirectory() {
    std::string assets_dir;

#ifdef _WIN32
    // Windows: Use %APPDATA%/autovibez/assets
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        assets_dir = std::string(appdata) + "/autovibez/assets";
    } else {
        assets_dir = "assets";  // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Application Support/autovibez/assets
    const char* home = std::getenv("HOME");
    if (home) {
        assets_dir = std::string(home) + "/Library/Application Support/autovibez/assets";
    } else {
        assets_dir = "assets";  // Fallback
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
            assets_dir = "assets";  // Last resort fallback
        }
    }
#endif

    if (!std::filesystem::exists(assets_dir)) {
        std::filesystem::create_directories(assets_dir);
    }

    return assets_dir;
}

void seedRand() {
#ifndef _WIN32
    srand((int)(time(NULL)));
#endif
}

void initGL() {
#ifdef USE_GLES
    // use GLES 3.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    // Disabling compatibility profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
}

void enableGLDebugOutput() {
#if OGL_DEBUG && !defined(USE_GLES)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugGL, NULL);
#endif
}

AutoVibezApp* setupSDLApp() {
    AutoVibezApp* app;
    seedRand();

    if (!initLoopback()) {
        SDL_Log("Failed to initialize audio loopback device.");
        exit(1);
    }

#ifdef SDL_HINT_AUDIO_INCLUDE_MONITORS
    SDL_SetHint(SDL_HINT_AUDIO_INCLUDE_MONITORS, "1");
#endif

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    if (!SDL_VERSION_ATLEAST(2, 0, 5)) {
        SDL_Log("SDL version 2.0.5 or greater is required. You have %i.%i.%i", SDL_MAJOR_VERSION, SDL_MINOR_VERSION,
                SDL_PATCHLEVEL);
        exit(1);
    }

    // Get display bounds for positioning
    SDL_Rect initialWindowBounds;
#if SDL_VERSION_ATLEAST(2, 0, 5)
    // new and better
    SDL_GetDisplayUsableBounds(0, &initialWindowBounds);
#else
    SDL_GetDisplayBounds(0, &initialWindowBounds);
#endif

    initGL();

    SDL_Window* win =
        SDL_CreateWindow("AutoVibez", 0, 0, Constants::DEFAULT_WINDOW_SIZE, Constants::DEFAULT_WINDOW_SIZE,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    int width, height;
    SDL_GL_GetDrawableSize(win, &width, &height);

    SDL_GLContext glCtx = SDL_GL_CreateContext(win);

#if defined(_WIN32)
    GLenum err = glewInit();
#endif /** _WIN32 */

    SDL_SetWindowTitle(win, "AutoVibez");

    SDL_GL_MakeCurrent(win, glCtx);           // associate GL context with main window
    int avsync = SDL_GL_SetSwapInterval(-1);  // try to enable adaptive vsync
    if (avsync == -1) {                       // adaptive vsync not supported
        SDL_GL_SetSwapInterval(1);            // enable updates synchronized with vertical retrace
    }

    std::string base_path = getAssetsDirectory();

    std::string configFilePath = findConfigFile();
    if (configFilePath.empty()) {
        // Continue with defaults instead of returning early
    }

    std::string presetURL = base_path + "/presets";
    std::string textureURL = base_path + "/textures";

    std::string xdg_assets = getAssetsDirectory();
    if (std::filesystem::exists(xdg_assets + "/presets")) {
        presetURL = xdg_assets + "/presets";
        textureURL = xdg_assets + "/textures";
    } else {
        std::string local_assets = "assets";
        if (std::filesystem::exists(local_assets + "/presets")) {
            presetURL = local_assets + "/presets";
            textureURL = local_assets + "/textures";
        }
    }

    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);
        std::string configPreset = config.getPresetPath();
        std::string configTexture = config.getTexturePath();

        if (!configPreset.empty()) {
            std::string expandedPreset = expandTilde(configPreset);
            if (std::filesystem::exists(expandedPreset)) {
                presetURL = expandedPreset;
            }
        }
        if (!configTexture.empty()) {
            std::string expandedTexture = expandTilde(configTexture);
            if (std::filesystem::exists(expandedTexture)) {
                textureURL = expandedTexture;
            }
        }
    }

    int audioDeviceIndex = 0;
    bool showFps = false;

    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);

        audioDeviceIndex = config.getAudioDeviceIndex();
        showFps = config.getShowFps();
    }

    app = new AutoVibezApp(glCtx, presetURL, textureURL, audioDeviceIndex, showFps);

    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);
        auto* projectMHandle = app->projectM();

        projectm_set_mesh_size(projectMHandle,
                               config.read<uint32_t>(StringConstants::MESH_X_KEY, Constants::DEFAULT_MESH_X),
                               config.read<uint32_t>(StringConstants::MESH_Y_KEY, Constants::DEFAULT_MESH_Y));

        // Get window size from config
        int configWidth = config.read<uint32_t>(StringConstants::WINDOW_WIDTH_KEY, Constants::DEFAULT_WINDOW_SIZE);
        int configHeight = config.read<uint32_t>(StringConstants::WINDOW_HEIGHT_KEY, Constants::DEFAULT_WINDOW_SIZE);
        SDL_SetWindowSize(win, configWidth, configHeight);
        projectm_set_soft_cut_duration(projectMHandle, config.read<double>("Smooth Preset Duration", 3));
        projectm_set_preset_duration(projectMHandle, config.read<double>(StringConstants::PRESET_DURATION_KEY,
                                                                         Constants::DEFAULT_PRESET_DURATION));
        projectm_set_easter_egg(projectMHandle, config.read<float>("Easter Egg Parameter", 0.0));
        projectm_set_hard_cut_enabled(projectMHandle, config.read<bool>("hard_cuts_enabled", false));
        projectm_set_hard_cut_duration(projectMHandle, config.read<double>(StringConstants::HARD_CUT_DURATION_KEY,
                                                                           Constants::DEFAULT_HARD_CUT_DURATION));
        projectm_set_hard_cut_sensitivity(projectMHandle, config.read<float>("hard_cut_sensitivity", 1.0));
        projectm_set_beat_sensitivity(projectMHandle, config.read<float>("beat_sensitivity", 1.0));
        projectm_set_aspect_correction(projectMHandle, config.read<bool>("Aspect Correction", true));
        projectm_set_fps(projectMHandle, config.read<int32_t>(StringConstants::FPS_KEY, Constants::DEFAULT_FPS_VALUE));

        // Handle fullscreen setting
        bool fullscreen = config.read<bool>("fullscreen", false);
        if (fullscreen) {
            SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else {
            // Center the window on screen
            SDL_SetWindowPosition(win, initialWindowBounds.x + (initialWindowBounds.w - configWidth) / 2,
                                  initialWindowBounds.y + (initialWindowBounds.h - configHeight) / 2);
        }
    }
    // Get the final window size for the resize call
    int finalWidth, finalHeight;
    SDL_GetWindowSize(win, &finalWidth, &finalHeight);
    app->resizeWindow(finalWidth, finalHeight);

    // Create a help menu specific to SDL
    // Note: modKey removed - not needed for current implementation

    app->initialize(win);

    // Synchronize fullscreen state after window initialization
    app->syncFullscreenState();

#if STEREOSCOPIC_SBS
    app->toggleFullScreen();
#endif
#if FAKE_AUDIO
    app->fakeAudio = true;
#endif

    enableGLDebugOutput();
    configureLoopback(app);

#if !FAKE_AUDIO
#ifdef _WIN32
// On Windows, use WASAPI if available, otherwise SDL
#ifdef WASAPI_LOOPBACK
    // WASAPI is handled in configureLoopback()
#else
    if (app->initializeAudioInput())
        app->beginAudioCapture();
#endif
#else
    // On Linux/macOS, always use SDL audio
    if (app->initializeAudioInput())
        app->beginAudioCapture();
#endif
#endif

    return app;
}
