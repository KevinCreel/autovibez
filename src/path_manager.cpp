#include "path_manager.hpp"
#include "constants.hpp"
#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>

std::string PathManager::getConfigDirectory() {
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
    config_dir = getXDGConfigHome() + "/autovibez/config";
#endif
    
    ensureDirectoryExists(config_dir);
    return config_dir;
}

std::string PathManager::getAssetsDirectory() {
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
    assets_dir = getXDGDataHome() + "/autovibez/assets";
#endif
    
    ensureDirectoryExists(assets_dir);
    return assets_dir;
}

std::string PathManager::getDataDirectory() {
    return getAssetsDirectory();
}

std::string PathManager::findConfigFile() {
    // Check environment variable first
    const char* config_env = std::getenv("AUTOVIBEZ_CONFIG");
    if (config_env) {
        return std::string(config_env);
    }
    
    // Check XDG config directory
    std::string xdg_config = getConfigDirectory() + "/config.inp";
    if (std::filesystem::exists(xdg_config)) {
        return xdg_config;
    }
    
    // Check local config directory
    if (std::filesystem::exists(Constants::DEFAULT_CONFIG_FILE)) {
        return Constants::DEFAULT_CONFIG_FILE;
    }
    
    return "";
}

std::string PathManager::expandTilde(const std::string& path) {
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

std::string PathManager::getCacheDirectory() {
    std::string cache_dir;
    
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        cache_dir = std::string(appdata) + "/autovibez/cache";
    } else {
        cache_dir = "build/data"; // Fallback
    }
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (home) {
        cache_dir = std::string(home) + "/Library/Caches/autovibez";
    } else {
        cache_dir = "build/data"; // Fallback
    }
#else
    // Linux/Unix: Use XDG cache directory
    const char* xdg_cache_home = std::getenv("XDG_CACHE_HOME");
    if (xdg_cache_home && strlen(xdg_cache_home) > 0) {
        cache_dir = std::string(xdg_cache_home) + "/autovibez";
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            cache_dir = std::string(home) + "/.cache/autovibez";
        } else {
            cache_dir = "build/data"; // Last resort fallback
        }
    }
#endif
    
    ensureDirectoryExists(cache_dir);
    return cache_dir;
}

std::string PathManager::getStateDirectory() {
    std::string state_dir;
    
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        state_dir = std::string(appdata) + "/autovibez/state";
    } else {
        state_dir = "build/data"; // Fallback
    }
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (home) {
        state_dir = std::string(home) + "/Library/Application Support/autovibez/state";
    } else {
        state_dir = "build/data"; // Fallback
    }
#else
    // Linux/Unix: Use XDG state directory
    const char* xdg_state_home = std::getenv("XDG_STATE_HOME");
    if (xdg_state_home && strlen(xdg_state_home) > 0) {
        state_dir = std::string(xdg_state_home) + "/autovibez";
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            state_dir = std::string(home) + "/.local/state/autovibez";
        } else {
            state_dir = "build/data"; // Last resort fallback
        }
    }
#endif
    
    ensureDirectoryExists(state_dir);
    return state_dir;
}

std::string PathManager::getXDGConfigHome() {
    const char* xdg_config_home = std::getenv("XDG_CONFIG_HOME");
    if (xdg_config_home && strlen(xdg_config_home) > 0) {
        return std::string(xdg_config_home);
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + "/.config";
        } else {
            return "config"; // Last resort fallback
        }
    }
}

std::string PathManager::getXDGDataHome() {
    const char* xdg_data_home = std::getenv("XDG_DATA_HOME");
    if (xdg_data_home && strlen(xdg_data_home) > 0) {
        return std::string(xdg_data_home);
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + "/.local/share";
        } else {
            return "assets"; // Last resort fallback
        }
    }
}

void PathManager::ensureDirectoryExists(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
} 