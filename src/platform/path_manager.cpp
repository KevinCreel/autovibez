#include "path_manager.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#include "constants.hpp"
#include "path_constants.hpp"
#include "string_utils.hpp"

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>
#endif

std::string PathManager::getConfigDirectory() {
    std::string config_dir;

#ifdef _WIN32
    // Windows: Use %APPDATA%/autovibez/config
    const char* appdata = std::getenv(PathConstants::ENV_APPDATA);
    if (appdata) {
        config_dir = joinPath(joinPath(std::string(appdata), PathConstants::APP_NAME), PathConstants::CONFIG_DIR);
    } else {
        config_dir = PathConstants::FALLBACK_CONFIG;  // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Application Support/autovibez/config
    const char* home = std::getenv(PathConstants::ENV_HOME);
    if (home) {
        config_dir =
            joinPath(joinPath(joinPath(std::string(home), "Library/Application Support"), PathConstants::APP_NAME),
                     PathConstants::CONFIG_DIR);
    } else {
        config_dir = PathConstants::FALLBACK_CONFIG;  // Fallback
    }
#else
    // Linux/Unix: Use XDG Base Directory Specification
    config_dir = joinPath(joinPath(getXDGConfigHome(), PathConstants::APP_NAME), PathConstants::CONFIG_DIR);
#endif

    ensureDirectoryExists(config_dir);
    return config_dir;
}

std::string PathManager::getAssetsDirectory() {
    std::string assets_dir;

#ifdef _WIN32
    // Windows: Use %APPDATA%/autovibez/assets
    const char* appdata = std::getenv(PathConstants::ENV_APPDATA);
    if (appdata) {
        assets_dir = joinPath(joinPath(std::string(appdata), PathConstants::APP_NAME), PathConstants::ASSETS_DIR);
    } else {
        assets_dir = PathConstants::FALLBACK_ASSETS;  // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Application Support/autovibez/assets
    const char* home = std::getenv(PathConstants::ENV_HOME);
    if (home) {
        assets_dir =
            joinPath(joinPath(joinPath(std::string(home), "Library/Application Support"), PathConstants::APP_NAME),
                     PathConstants::ASSETS_DIR);
    } else {
        assets_dir = PathConstants::FALLBACK_ASSETS;  // Fallback
    }
#else
    // Linux/Unix: Use XDG Base Directory Specification
    assets_dir = joinPath(joinPath(getXDGDataHome(), PathConstants::APP_NAME), PathConstants::ASSETS_DIR);
#endif

    ensureDirectoryExists(assets_dir);
    return assets_dir;
}

std::string PathManager::getDataDirectory() {
    std::string data_dir;

#ifdef _WIN32
    // Windows: Use %APPDATA%/autovibez
    const char* appdata = std::getenv(PathConstants::ENV_APPDATA);
    if (appdata) {
        data_dir = joinPath(std::string(appdata), PathConstants::APP_NAME);
    } else {
        data_dir = PathConstants::FALLBACK_DATA;  // Fallback
    }
#elif defined(__APPLE__)
    // macOS: Use ~/Library/Application Support/autovibez
    const char* home = std::getenv(PathConstants::ENV_HOME);
    if (home) {
        data_dir = joinPath(joinPath(std::string(home), "Library/Application Support"), PathConstants::APP_NAME);
    } else {
        data_dir = PathConstants::FALLBACK_DATA;  // Fallback
    }
#else
    // Linux/Unix: Use XDG Base Directory Specification
    data_dir = joinPath(getXDGDataHome(), PathConstants::APP_NAME);
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
    const char* userprofile = std::getenv(PathConstants::ENV_USERPROFILE);
    if (userprofile) {
        return std::string(userprofile) + path.substr(1);
    }
    const char* home = std::getenv(PathConstants::ENV_HOME);
    if (home) {
        return std::string(home) + path.substr(1);
    }
#else
    // Unix-like systems: Use $HOME
    const char* home = std::getenv(PathConstants::ENV_HOME);
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
            cache_dir = joinPath(joinPath(appdata, PathConstants::APP_NAME), PathConstants::CACHE_DIR);
        } else {
            cache_dir = PathConstants::FALLBACK_CACHE;  // Fallback
        }
    } else if (isMacOS()) {
        std::string caches = getMacOSCaches();
        if (!caches.empty()) {
            cache_dir = joinPath(caches, PathConstants::APP_NAME);
        } else {
            cache_dir = PathConstants::FALLBACK_CACHE;  // Fallback
        }
    } else {
        // Linux/Unix: Use XDG cache directory
        cache_dir = joinPath(getXDGCacheHome(), PathConstants::APP_NAME);
    }

    ensureDirectoryExists(cache_dir);
    return cache_dir;
}

std::string PathManager::getStateDirectory() {
    std::string state_dir;

    if (isWindows()) {
        std::string appdata = getWindowsAppData();
        if (!appdata.empty()) {
            state_dir = joinPath(joinPath(appdata, PathConstants::APP_NAME), PathConstants::STATE_DIR);
        } else {
            state_dir = PathConstants::FALLBACK_STATE;  // Fallback
        }
    } else if (isMacOS()) {
        std::string appsupport = getMacOSAppSupport();
        if (!appsupport.empty()) {
            state_dir = joinPath(joinPath(appsupport, PathConstants::APP_NAME), PathConstants::STATE_DIR);
        } else {
            state_dir = PathConstants::FALLBACK_STATE;  // Fallback
        }
    } else {
        // Linux/Unix: Use XDG state directory
        state_dir = joinPath(getXDGStateHome(), PathConstants::APP_NAME);
    }

    ensureDirectoryExists(state_dir);
    return state_dir;
}

std::string PathManager::getXDGConfigHome() {
    const char* xdg_config_home = std::getenv(PathConstants::ENV_XDG_CONFIG_HOME);
    if (xdg_config_home && strlen(xdg_config_home) > 0) {
        return std::string(xdg_config_home);
    } else {
        const char* home = std::getenv(PathConstants::ENV_HOME);
        if (home) {
            return joinPath(std::string(home),
                            std::string(PathConstants::XDG_CONFIG).substr(1));  // Remove leading slash
        } else {
            return PathConstants::FALLBACK_CONFIG;  // Last resort fallback
        }
    }
}

std::string PathManager::getXDGDataHome() {
    const char* xdg_data_home = std::getenv(PathConstants::ENV_XDG_DATA_HOME);
    if (xdg_data_home && strlen(xdg_data_home) > 0) {
        return std::string(xdg_data_home);
    } else {
        const char* home = std::getenv(PathConstants::ENV_HOME);
        if (home) {
            return joinPath(std::string(home),
                            std::string(PathConstants::XDG_LOCAL_SHARE).substr(1));  // Remove leading slash
        } else {
            return PathConstants::FALLBACK_ASSETS;  // Last resort fallback
        }
    }
}

void PathManager::ensureDirectoryExists(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
}

std::string PathManager::getDatabasePath() {
    return joinPath(getStateDirectory(), PathConstants::DATABASE_FILE);
}

std::string PathManager::getMixesDirectory() {
    return joinPath(getDataDirectory(), PathConstants::MIXES_DIR);
}

std::string PathManager::getFileMappingsPath() {
    return joinPath(getStateDirectory(), PathConstants::FILE_MAPPINGS_FILE);
}

std::string PathManager::getPresetsDirectory() {
    return joinPath(getAssetsDirectory(), PathConstants::PRESETS_DIR);
}

std::string PathManager::getTexturesDirectory() {
    return joinPath(getAssetsDirectory(), PathConstants::TEXTURES_DIR);
}

std::vector<std::string> PathManager::getConfigFileSearchPaths() {
    std::vector<std::string> paths;

    // 1. Environment variable override
    const char* config_env = std::getenv(PathConstants::ENV_AUTOVIBEZ_CONFIG);
    if (config_env) {
        paths.push_back(config_env);
    }

    // 2. XDG config directory
    paths.push_back(joinPath(getConfigDirectory(), PathConstants::CONFIG_FILE));

    // 3. Platform-specific system-wide fallback
    if (isWindows()) {
        // Windows: Use system-wide installation if available
        paths.push_back(
            joinPath(joinPath(getWindowsProgramData(), PathConstants::APP_NAME), PathConstants::CONFIG_FILE));
    } else if (isMacOS()) {
        // macOS: Use system-wide installation if available
        paths.push_back(
            joinPath(joinPath("/Library/Application Support", PathConstants::APP_NAME), PathConstants::CONFIG_FILE));
    } else {
        // Linux/Unix: Use XDG data directories for system-wide installation
        auto xdg_dirs = getXDGDataDirectories();
        for (const auto& dir : xdg_dirs) {
            paths.push_back(joinPath(joinPath(dir, PathConstants::APP_NAME), PathConstants::CONFIG_FILE));
        }
    }

    // 4. Local config directory
    paths.push_back(joinPath(PathConstants::CONFIG_DIR, PathConstants::CONFIG_FILE));

    return paths;
}

std::vector<std::string> PathManager::getPresetSearchPaths() {
    std::vector<std::string> paths;

    // 1. XDG data directory
    paths.push_back(getPresetsDirectory());

    // 2. Platform-specific system-wide fallback
    if (isWindows()) {
        // Windows: Use system-wide installation if available
        paths.push_back(
            joinPath(joinPath(getWindowsProgramData(), PathConstants::APP_NAME), PathConstants::PRESETS_DIR));
    } else if (isMacOS()) {
        // macOS: Use system-wide installation if available
        paths.push_back(
            joinPath(joinPath("/Library/Application Support", PathConstants::APP_NAME), PathConstants::PRESETS_DIR));
    } else {
        // Linux/Unix: Use XDG data directories for system-wide installation
        auto xdg_dirs = getXDGDataDirectories();
        for (const auto& dir : xdg_dirs) {
            paths.push_back(joinPath(joinPath(dir, PathConstants::APP_NAME), PathConstants::PRESETS_DIR));
        }
    }

    // 3. Local assets
    paths.push_back(joinPath(PathConstants::ASSETS_DIR, PathConstants::PRESETS_DIR));

    return paths;
}

std::vector<std::string> PathManager::getTextureSearchPaths() {
    std::vector<std::string> paths;

    // 1. XDG data directory
    paths.push_back(getTexturesDirectory());

    // 2. Platform-specific system-wide fallback
    if (isWindows()) {
        // Windows: Use system-wide installation if available
        paths.push_back(
            joinPath(joinPath(getWindowsProgramData(), PathConstants::APP_NAME), PathConstants::TEXTURES_DIR));
    } else if (isMacOS()) {
        // macOS: Use system-wide installation if available
        paths.push_back(
            joinPath(joinPath("/Library/Application Support", PathConstants::APP_NAME), PathConstants::TEXTURES_DIR));
    } else {
        // Linux/Unix: Use XDG data directories for system-wide installation
        auto xdg_dirs = getXDGDataDirectories();
        for (const auto& dir : xdg_dirs) {
            paths.push_back(joinPath(joinPath(dir, PathConstants::APP_NAME), PathConstants::TEXTURES_DIR));
        }
    }

    // 3. Local assets
    paths.push_back(joinPath(PathConstants::ASSETS_DIR, PathConstants::TEXTURES_DIR));

    return paths;
}

bool PathManager::pathExists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string PathManager::getAppName() {
    return PathConstants::APP_NAME;
}

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

std::string PathManager::getXDGCacheHome() {
    const char* xdg_cache_home = std::getenv(PathConstants::ENV_XDG_CACHE_HOME);
    if (xdg_cache_home && strlen(xdg_cache_home) > 0) {
        return std::string(xdg_cache_home);
    } else {
        const char* home = std::getenv(PathConstants::ENV_HOME);
        if (home) {
            return joinPath(std::string(home),
                            std::string(PathConstants::XDG_CACHE).substr(1));  // Remove leading slash
        } else {
            return PathConstants::FALLBACK_CACHE;  // Last resort fallback
        }
    }
}

std::string PathManager::getXDGStateHome() {
    const char* xdg_state_home = std::getenv(PathConstants::ENV_XDG_STATE_HOME);
    if (xdg_state_home && strlen(xdg_state_home) > 0) {
        return std::string(xdg_state_home);
    } else {
        const char* home = std::getenv(PathConstants::ENV_HOME);
        if (home) {
            return joinPath(std::string(home),
                            std::string(PathConstants::XDG_STATE).substr(1));  // Remove leading slash
        } else {
            return PathConstants::FALLBACK_STATE;  // Last resort fallback
        }
    }
}

std::vector<std::string> PathManager::getXDGDataDirectories() {
    std::vector<std::string> directories;

    // Add XDG_DATA_HOME
    directories.push_back(getXDGDataHome());

    // Add XDG_DATA_DIRS (split by colon)
    const char* xdg_data_dirs = std::getenv(PathConstants::ENV_XDG_DATA_DIRS);
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
            directories.push_back(getWindowsProgramData());
            directories.push_back(joinPath(getWindowsProgramFiles(), PathConstants::APP_NAME));
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

std::string PathManager::getWindowsAppData() {
    const char* appdata = std::getenv(PathConstants::ENV_APPDATA);
    if (appdata) {
        return std::string(appdata);
    } else {
        return "";  // Fallback handled by caller
    }
}

std::string PathManager::getMacOSAppSupport() {
    const char* home = std::getenv(PathConstants::ENV_HOME);
    if (home) {
        return joinPath(std::string(home), "Library/Application Support");
    } else {
        return "";  // Fallback handled by caller
    }
}

std::string PathManager::getMacOSCaches() {
    const char* home = std::getenv(PathConstants::ENV_HOME);
    if (home) {
        return joinPath(std::string(home), "Library/Caches");
    } else {
        return "";  // Fallback handled by caller
    }
}

std::string PathManager::getWindowsProgramData() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, buffer))) {
        return std::string(buffer);
    }
    // Fallback to environment variable
    const char* programdata = std::getenv("PROGRAMDATA");
    if (programdata) {
        return std::string(programdata);
    }
#endif
    return "C:/ProgramData";  // Last resort fallback
}

std::string PathManager::getWindowsProgramFiles() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, buffer))) {
        return std::string(buffer);
    }
    // Fallback to environment variable
    const char* programfiles = std::getenv("PROGRAMFILES");
    if (programfiles) {
        return std::string(programfiles);
    }
#endif
    return "C:/Program Files";  // Last resort fallback
}

// ===== Internal Utilities =====

std::string PathManager::getHomeDirectory() {
    const char* home = std::getenv(PathConstants::ENV_HOME);
    if (home) {
        return std::string(home);
    }

#ifdef _WIN32
    const char* userprofile = std::getenv(PathConstants::ENV_USERPROFILE);
    if (userprofile) {
        return std::string(userprofile);
    }
#endif

    return "";  // Fallback handled by caller
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

    return normalizePath(result + component);
}

std::string PathManager::normalizePath(const std::string& path) {
    std::string normalized = path;

#ifdef _WIN32
    // Convert forward slashes to backslashes on Windows
    normalized = AutoVibez::Utils::StringUtils::replaceChar(normalized, '/', '\\');
#else
    // Convert backslashes to forward slashes on Unix-like systems
    normalized = AutoVibez::Utils::StringUtils::replaceChar(normalized, '\\', '/');
#endif

    return normalized;
}