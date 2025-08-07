#pragma once

/**
 * @brief Centralized constants for all path-related values
 * 
 * This eliminates hardcoded paths scattered throughout PathManager
 */
namespace PathConstants {
    // Application name
    constexpr const char* APP_NAME = "autovibez";
    
    // Directory names
    constexpr const char* CONFIG_DIR = "config";
    constexpr const char* ASSETS_DIR = "assets";
    constexpr const char* DATA_DIR = "data";
    constexpr const char* CACHE_DIR = "cache";
    constexpr const char* STATE_DIR = "state";
    constexpr const char* MIXES_DIR = "mixes";
    constexpr const char* PRESETS_DIR = "presets";
    constexpr const char* TEXTURES_DIR = "textures";
    
    // File names
    constexpr const char* CONFIG_FILE = "config.inp";
    constexpr const char* DATABASE_FILE = "autovibez_mixes.db";
    constexpr const char* FILE_MAPPINGS_FILE = "file_mappings.txt";
    
    
    // Environment variables
    constexpr const char* ENV_HOME = "HOME";
    constexpr const char* ENV_USERPROFILE = "USERPROFILE";
    constexpr const char* ENV_APPDATA = "APPDATA";
    constexpr const char* ENV_XDG_CONFIG_HOME = "XDG_CONFIG_HOME";
    constexpr const char* ENV_XDG_DATA_HOME = "XDG_DATA_HOME";
    constexpr const char* ENV_XDG_DATA_DIRS = "XDG_DATA_DIRS";
    constexpr const char* ENV_XDG_CACHE_HOME = "XDG_CACHE_HOME";
    constexpr const char* ENV_XDG_STATE_HOME = "XDG_STATE_HOME";
    constexpr const char* ENV_AUTOVIBEZ_CONFIG = "AUTOVIBEZ_CONFIG";
    
    // Fallback paths
    constexpr const char* FALLBACK_CONFIG = "config";
    constexpr const char* FALLBACK_ASSETS = "assets";
    constexpr const char* FALLBACK_DATA = "data";
    constexpr const char* FALLBACK_CACHE = "cache";
    constexpr const char* FALLBACK_STATE = "state";
    
    // XDG standard paths
    constexpr const char* XDG_LOCAL_SHARE = "/.local/share";
    constexpr const char* XDG_CONFIG = "/.config";
    constexpr const char* XDG_CACHE = "/.cache";
    constexpr const char* XDG_STATE = "/.local/state";
}
