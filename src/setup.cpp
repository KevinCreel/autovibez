#include "setup.hpp"
#include "autovibez_app.hpp"
#include "config_manager.hpp"
#include <SDL2/SDL.h>
#include <iostream>
#include <filesystem>

#include <SDL2/SDL_hints.h>

#include <chrono>
#include <cmath>
#include <vector>
#include <cstdlib>

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
std::string getConfigFilePath(std::string datadir_path) {
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

// Find config file
std::string findConfigFile() {
    // Check for environment variable first
    const char* config_env = std::getenv("AUTOVIBEZ_CONFIG");
    if (config_env && std::filesystem::exists(config_env)) {
        return config_env;
    }
    
    // Look for config in standard location
    if (std::filesystem::exists("config/config.inp")) {
        return "config/config.inp";
    }
    
    return "";
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

    std::string base_path = "/usr/local/share/autovibez";

    // load configuration file - ONLY use local config
    std::string configFilePath = findConfigFile();
    if (configFilePath.empty()) {
        printf("⚠️  Local config.inp not found, using defaults\n");
    }
    
    // Use local assets directory if it exists, otherwise fall back to system paths
    std::string presetURL = base_path + "/presets";
    std::string textureURL = base_path + "/textures";
    
    // Check if local assets directory exists
    std::string local_assets = "assets";
    if (std::filesystem::exists(local_assets + "/presets")) {
        presetURL = local_assets + "/presets";
        textureURL = local_assets + "/textures";
    }

    if (!configFilePath.empty()) {
        ConfigFile config(configFilePath);
        std::string configPreset = config.getPresetPath();
        std::string configTexture = config.getTexturePath();
        if (!configPreset.empty()) presetURL = configPreset;
        if (!configTexture.empty()) textureURL = configTexture;
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
        projectm_set_hard_cut_enabled(projectMHandle,  config.read<bool>("Hard Cuts Enabled", false));
        projectm_set_hard_cut_duration(projectMHandle, config.read<double>("Hard Cut Duration", 60));
        projectm_set_hard_cut_sensitivity(projectMHandle, config.read<float>("Hard Cut Sensitivity", 1.0));
        projectm_set_beat_sensitivity(projectMHandle, config.read<float>("Beat Sensitivity", 1.0));
        projectm_set_aspect_correction(projectMHandle, config.read<bool>("Aspect Correction", true));
        projectm_set_fps(projectMHandle, config.read<int32_t>("FPS", 60));

        app->setFps(config.read<uint32_t>("FPS", 60));
        
        // Read new config settings
        audioDeviceIndex = config.getAudioDeviceIndex();
        showFps = config.getShowFps();
        
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
    std::string modKey = "CTRL";

#if __APPLE__
    modKey = "CMD";
#endif

    app->init(win);

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
