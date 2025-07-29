#include "setup.hpp"
#include "autovibez_app.hpp"
#include "config_manager.hpp"
#include <SDL2/SDL.h>
#include <iostream>
#include <filesystem>
#include <cstdlib>

#include <SDL2/SDL_hints.h>

#include <chrono>
#include <cmath>
#include <vector>
#include <cstdlib>

// Helper function to expand tilde in paths (cross-platform)
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
    
    return path; // Can't expand, return as-is
}

#if OGL_DEBUG
void debugGL(GLenum source,
             GLenum type,
             GLuint id,
             GLenum severity,
             GLsizei length,
             const GLchar* message,
             const void* userParam) {

    /*if (type != GL_DEBUG_TYPE_OTHER)*/
    {
        std::cerr << " -- \n" << "Type: " <<
        type << "; Source: " <<
        source <<"; ID: " << id << "; Severity: " <<
        severity << "\n" << message << "\n";
    }
}
#endif

// return path to config file to use
std::string getConfigFilePath(const std::string& datadir_path) {
    std::string config_path = datadir_path + "/config.inp";
    
    // Check if the specified config file exists
    std::ifstream f_config(config_path);
    if (f_config.good()) {
        return config_path;
    }
    
    // If not found, return empty string to indicate no config
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
            config_dir = Constants::DEFAULT_CONFIG_FILE; // Last resort fallback
        }
    }
#endif
    
    // Create directory if it doesn't exist
    if (!std::filesystem::exists(config_dir)) {
        std::filesystem::create_directories(config_dir);
    }
    
    return config_dir;
}

// Find config file
std::string findConfigFile() {
    // Check for environment variable first
    const char* config_env = std::getenv("AUTOVIBEZ_CONFIG");
    if (config_env && std::filesystem::exists(config_env)) {
        return config_env;
    }
    
    // Look for config in XDG config directory
    std::string xdg_config_path = getConfigDirectory() + "/config.inp";
    if (std::filesystem::exists(xdg_config_path)) {
        return xdg_config_path;
    }
    
    // Fallback to local config directory
    if (std::filesystem::exists(Constants::DEFAULT_CONFIG_FILE)) {
        return Constants::DEFAULT_CONFIG_FILE;
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

void dumpOpenGLInfo() {
}

void initStereoscopicView(SDL_Window *win) {
    (void)win; // Parameter not used in current implementation
#if STEREOSCOPIC_SB
    // enable stereo
    if (SDL_GL_SetAttribute(SDL_GL_STEREO, 1) == 0)
    {
        SDL_Log("SDL_GL_STEREO: true");
    }

    // requires fullscreen mode
    SDL_ShowCursor(false);
    SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN);
#endif
}

void enableGLDebugOutput() {
#if OGL_DEBUG && !defined (USE_GLES)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugGL, NULL);
#endif
}

// initialize SDL, openGL, config
AutoVibezApp *setupSDLApp() {
    AutoVibezApp *app;
    seedRand();
        
    if (!initLoopback())
		{
			SDL_Log("Failed to initialize audio loopback device.");
			exit(1);
		}

#if UNLOCK_FPS
    setenv("vblank_mode", "0", 1);
#endif

#ifdef SDL_HINT_AUDIO_INCLUDE_MONITORS
    SDL_SetHint(SDL_HINT_AUDIO_INCLUDE_MONITORS, "1");
#endif

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    if (! SDL_VERSION_ATLEAST(2, 0, 5)) {
        SDL_Log("SDL version 2.0.5 or greater is required. You have %i.%i.%i", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
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

    // Create window with default size, will be updated from config
    SDL_Window *win = SDL_CreateWindow("AutoVibez", 0, 0, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    int width, height;
    SDL_GL_GetDrawableSize(win,&width,&height);

    initStereoscopicView(win);

    SDL_GLContext glCtx = SDL_GL_CreateContext(win);

#if defined(_WIN32)
	GLenum err = glewInit();
#endif /** _WIN32 */

    dumpOpenGLInfo();

    SDL_SetWindowTitle(win, "AutoVibez");

    SDL_GL_MakeCurrent(win, glCtx);  // associate GL context with main window
    int avsync = SDL_GL_SetSwapInterval(-1); // try to enable adaptive vsync
    if (avsync == -1) { // adaptive vsync not supported
        SDL_GL_SetSwapInterval(1); // enable updates synchronized with vertical retrace
    }

    std::string base_path = getAssetsDirectory();

    // load configuration file - use XDG config directory
    std::string configFilePath = findConfigFile();
    if (configFilePath.empty()) {
        printf("⚠️  Config file not found, using defaults\n");
    }
    
    // Use XDG assets directory if it exists, otherwise fall back to system paths
    std::string presetURL = base_path + "/presets";
    std::string textureURL = base_path + "/textures";
    
    // Check if XDG assets directory exists
    std::string xdg_assets = getAssetsDirectory();
    if (std::filesystem::exists(xdg_assets + "/presets")) {
        presetURL = xdg_assets + "/presets";
        textureURL = xdg_assets + "/textures";
    } else {
        // Fallback to local assets directory if it exists
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
        
        // Only use config paths if they actually exist
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

    // Default values for new config settings
    int audioDeviceIndex = 0;
    bool showFps = false;

    app = new AutoVibezApp(glCtx, presetURL, textureURL, audioDeviceIndex, showFps);

    if (! configFilePath.empty())
    {
        // found config file, load it
        ConfigFile config(configFilePath);
        auto* projectMHandle = app->projectM();

        projectm_set_mesh_size(projectMHandle, config.read<uint32_t>("Mesh X", 32), config.read<uint32_t>("Mesh Y", 24));
        
        // Get window size from config
        int configWidth = config.read<uint32_t>("Window Width", 512);
        int configHeight = config.read<uint32_t>("Window Height", 512);
        SDL_SetWindowSize(win, configWidth, configHeight);
        projectm_set_soft_cut_duration(projectMHandle, config.read<double>("Smooth Preset Duration", config.read<int>("Smooth Transition Duration", 3)));
        projectm_set_preset_duration(projectMHandle, config.read<double>("Preset Duration", 30));
        projectm_set_easter_egg(projectMHandle, config.read<float>("Easter Egg Parameter", 0.0));
        projectm_set_hard_cut_enabled(projectMHandle,  config.read<bool>("hard_cuts_enabled", false));
        projectm_set_hard_cut_duration(projectMHandle, config.read<double>("Hard Cut Duration", 60));
        projectm_set_hard_cut_sensitivity(projectMHandle, config.read<float>("hard_cut_sensitivity", 1.0));
        projectm_set_beat_sensitivity(projectMHandle, config.read<float>("beat_sensitivity", 1.0));
        projectm_set_aspect_correction(projectMHandle, config.read<bool>("Aspect Correction", true));
        projectm_set_fps(projectMHandle, config.read<int32_t>("FPS", 60));

        app->setFps(config.read<uint32_t>("FPS", 60));
        
        // Read new config settings (used in app initialization)
        int audioDeviceIndex = config.getAudioDeviceIndex();
        bool showFps = config.getShowFps();
        
        // Handle fullscreen setting
        bool fullscreen = config.read<bool>("fullscreen", false);
        if (fullscreen) {
            SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else {
            // Center the window on screen
            SDL_SetWindowPosition(win, 
                initialWindowBounds.x + (initialWindowBounds.w - configWidth) / 2,
                initialWindowBounds.y + (initialWindowBounds.h - configHeight) / 2);
        }
    }
    // Get the final window size for the resize call
    int finalWidth, finalHeight;
    SDL_GetWindowSize(win, &finalWidth, &finalHeight);
    app->resize(finalWidth, finalHeight);

    // Create a help menu specific to SDL
    // Note: modKey is currently unused but kept for future help menu implementation
    std::string modKey = "CTRL";

#if __APPLE__
    modKey = "CMD";
#endif

    app->init(win);
    
    // Synchronize fullscreen state after window initialization
    app->syncFullscreenState();

#if STEREOSCOPIC_SBS
    app->toggleFullScreen();
#endif
#if FAKE_AUDIO
    app->fakeAudio  = true;
#endif

    enableGLDebugOutput();
    configureLoopback(app);

#if !FAKE_AUDIO && !WASAPI_LOOPBACK
    // get an audio input device
    if (app->initAudioInput())
        app->beginAudioCapture();
#endif

#if TEST_ALL_PRESETS
    testAllPresets(app);
    return 0;
#endif

    return app;
}

int64_t startUnlockedFPSCounter() {
	using namespace std::chrono;
	auto currentTime = steady_clock::now();
	// auto currentTimeMs = time_point_cast<milliseconds>(currentTime); // Unused variable
	auto elapsedMs = currentTime.time_since_epoch();

	return elapsedMs.count();
}

void advanceUnlockedFPSCounterFrame(int64_t startFrame) {
    static int32_t frameCount = 0;

    frameCount++;
	auto currentElapsedMs = startUnlockedFPSCounter();
	if (currentElapsedMs - startFrame > 5000)
	{
        printf("Frames[%d]\n", frameCount);
        exit(0);
    }
}
