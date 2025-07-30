#include "path_manager.hpp"
#include "constants.hpp"
#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <algorithm>

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
    data_dir = getXDGDataHome() + "/autovibez";
#endif
    
    ensureDirectoryExists(data_dir);
    return data_dir;
}

std::string PathManager::findConfigFile() {
    std::vector<std::string> search_paths = getConfigFileSearchPaths();
    
    for (const auto& path : search_paths) {
        if (pathExists(path)) {
            return path;
        }
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
    
    return path;
}

std::string PathManager::getCacheDirectory() {
    std::string cache_dir;
    
    if (isWindows()) {
        std::string appdata = getWindowsAppData();
        if (!appdata.empty()) {
            cache_dir = joinPath(appdata, "autovibez/cache");
        } else {
            cache_dir = "build/data"; // Fallback
        }
    } else if (isMacOS()) {
        std::string caches = getMacOSCaches();
        if (!caches.empty()) {
            cache_dir = joinPath(caches, "autovibez");
        } else {
            cache_dir = "build/data"; // Fallback
        }
    } else {
        // Linux/Unix: Use XDG cache directory
        cache_dir = joinPath(getXDGCacheHome(), "autovibez");
    }
    
    ensureDirectoryExists(cache_dir);
    return cache_dir;
}

std::string PathManager::getStateDirectory() {
    std::string state_dir;
    
    if (isWindows()) {
        std::string appdata = getWindowsAppData();
        if (!appdata.empty()) {
            state_dir = joinPath(appdata, "autovibez/state");
        } else {
            state_dir = "build/data"; // Fallback
        }
    } else if (isMacOS()) {
        std::string appsupport = getMacOSAppSupport();
        if (!appsupport.empty()) {
            state_dir = joinPath(appsupport, "autovibez/state");
        } else {
            state_dir = "build/data"; // Fallback
        }
    } else {
        // Linux/Unix: Use XDG state directory
        state_dir = joinPath(getXDGStateHome(), "autovibez");
    }
    
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

// ===== File Path Methods =====

std::string PathManager::getDatabasePath() {
    return getStateDirectory() + "/autovibez_mixes.db";
}

std::string PathManager::getMixesDirectory() {
    return getDataDirectory() + "/mixes";
}

std::string PathManager::getPresetsDirectory() {
    return getAssetsDirectory() + "/presets";
}

std::string PathManager::getTexturesDirectory() {
    return getAssetsDirectory() + "/textures";
}

// ===== Utility Methods =====

std::vector<std::string> PathManager::getConfigFileSearchPaths() {
    std::vector<std::string> paths;
    
    // 1. Environment variable override
    const char* config_env = std::getenv("AUTOVIBEZ_CONFIG");
    if (config_env) {
        paths.push_back(config_env);
    }
    
    // 2. XDG config directory
    paths.push_back(getConfigDirectory() + "/config.inp");
    
    // 3. Platform-specific system-wide fallback
    if (isWindows()) {
        // Windows: Use system-wide installation if available
        paths.push_back("C:/ProgramData/autovibez/config.inp");
    } else if (isMacOS()) {
        // macOS: Use system-wide installation if available
        paths.push_back("/Library/Application Support/autovibez/config.inp");
    } else {
        // Linux/Unix: Use system-wide installation if available
        paths.push_back("/usr/local/share/autovibez/config.inp");
        paths.push_back("/usr/share/autovibez/config.inp");
    }
    
    // 4. Local config directory
    paths.push_back(Constants::DEFAULT_CONFIG_FILE);
    
    return paths;
}

std::vector<std::string> PathManager::getPresetSearchPaths() {
    std::vector<std::string> paths;
    
    // 1. XDG data directory
    paths.push_back(getPresetsDirectory());
    
    // 2. Platform-specific system-wide fallback
    if (isWindows()) {
        // Windows: Use system-wide installation if available
        paths.push_back("C:/ProgramData/autovibez/presets");
    } else if (isMacOS()) {
        // macOS: Use system-wide installation if available
        paths.push_back("/Library/Application Support/autovibez/presets");
    } else {
        // Linux/Unix: Use system-wide installation if available
        paths.push_back("/usr/local/share/autovibez/presets");
        paths.push_back("/usr/share/autovibez/presets");
    }
    
    // 3. Local assets
    paths.push_back(Constants::DEFAULT_PRESET_PATH);
    
    return paths;
}

std::vector<std::string> PathManager::getTextureSearchPaths() {
    std::vector<std::string> paths;
    
    // 1. XDG data directory
    paths.push_back(getTexturesDirectory());
    
    // 2. Platform-specific system-wide fallback
    if (isWindows()) {
        // Windows: Use system-wide installation if available
        paths.push_back("C:/ProgramData/autovibez/textures");
    } else if (isMacOS()) {
        // macOS: Use system-wide installation if available
        paths.push_back("/Library/Application Support/autovibez/textures");
    } else {
        // Linux/Unix: Use system-wide installation if available
        paths.push_back("/usr/local/share/autovibez/textures");
        paths.push_back("/usr/share/autovibez/textures");
    }
    
    // 3. Local assets
    paths.push_back(Constants::DEFAULT_TEXTURE_PATH);
    
    return paths;
}

bool PathManager::pathExists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string PathManager::getAppName() {
    return "autovibez";
}

// ===== Platform Detection =====

bool PathManager::isLinux() {
#ifdef _WIN32
    return false;
#elif defined(__APPLE__)
    return false;
#else
    return true;
#endif
}

bool PathManager::isMacOS() {
#ifdef __APPLE__
    return true;
#else
    return false;
#endif
}

bool PathManager::isWindows() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

// ===== XDG Base Directory Methods =====

std::string PathManager::getXDGCacheHome() {
    const char* xdg_cache_home = std::getenv("XDG_CACHE_HOME");
    if (xdg_cache_home && strlen(xdg_cache_home) > 0) {
        return std::string(xdg_cache_home);
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + "/.cache";
        } else {
            return "cache"; // Last resort fallback
        }
    }
}

std::string PathManager::getXDGStateHome() {
    const char* xdg_state_home = std::getenv("XDG_STATE_HOME");
    if (xdg_state_home && strlen(xdg_state_home) > 0) {
        return std::string(xdg_state_home);
    } else {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::string(home) + "/.local/state";
        } else {
            return "state"; // Last resort fallback
        }
    }
}

std::vector<std::string> PathManager::getXDGDataDirectories() {
    std::vector<std::string> directories;
    
    // Add XDG_DATA_HOME
    directories.push_back(getXDGDataHome());
    
    // Add XDG_DATA_DIRS (split by colon)
    const char* xdg_data_dirs = std::getenv("XDG_DATA_DIRS");
    if (xdg_data_dirs) {
        std::string dirs_str(xdg_data_dirs);
        size_t pos = 0;
        while ((pos = dirs_str.find(':')) != std::string::npos) {
            directories.push_back(dirs_str.substr(0, pos));
            dirs_str.erase(0, pos + 1);
        }
        if (!dirs_str.empty()) {
            directories.push_back(dirs_str);
        }
    } else {
        // Platform-specific default data directories
        if (isWindows()) {
            // Windows: Use ProgramData and Program Files
            directories.push_back("C:/ProgramData");
            directories.push_back("C:/Program Files/autovibez");
        } else if (isMacOS()) {
            // macOS: Use system-wide Application Support
            directories.push_back("/Library/Application Support");
        } else {
            // Linux/Unix: Use standard system directories
            directories.push_back("/usr/local/share");
            directories.push_back("/usr/share");
        }
    }
    
    return directories;
}

// ===== Platform-Specific Methods =====

std::string PathManager::getWindowsAppData() {
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        return std::string(appdata);
    } else {
        return ""; // Fallback handled by caller
    }
}

std::string PathManager::getMacOSAppSupport() {
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/Library/Application Support";
    } else {
        return ""; // Fallback handled by caller
    }
}

std::string PathManager::getMacOSCaches() {
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/Library/Caches";
    } else {
        return ""; // Fallback handled by caller
    }
}

// ===== Internal Utilities =====

std::string PathManager::getHomeDirectory() {
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home);
    }
    
#ifdef _WIN32
    const char* userprofile = std::getenv("USERPROFILE");
    if (userprofile) {
        return std::string(userprofile);
    }
#endif
    
    return ""; // Fallback handled by caller
}

std::string PathManager::joinPath(const std::string& base, const std::string& component) {
    if (base.empty()) {
        return component;
    }
    if (component.empty()) {
        return base;
    }
    
    // Handle trailing slash in base
    std::string result = base;
    if (result.back() != '/' && result.back() != '\\') {
        result += '/';
    }
    
    return result + component;
}

std::string PathManager::normalizePath(const std::string& path) {
    std::string normalized = path;
    
#ifdef _WIN32
    // Convert forward slashes to backslashes on Windows
    std::replace(normalized.begin(), normalized.end(), '/', '\\');
#else
    // Convert backslashes to forward slashes on Unix-like systems
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
#endif
    
    return normalized;
} 