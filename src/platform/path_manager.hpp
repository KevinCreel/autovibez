#pragma once

#include <filesystem>
#include <string>
#include <vector>

/**
 * Single source of truth for all directory and path management.
 * Provides XDG-compliant, cross-platform directory resolution.
 */
class PathManager {
public:
    /**
     * Get the XDG config directory for autovibez (cross-platform)
     */
    static std::string getConfigDirectory();

    /**
     * Get the XDG assets directory for autovibez (cross-platform)
     */
    static std::string getAssetsDirectory();

    /**
     * Get the data directory path
     */
    static std::string getDataDirectory();

    /**
     * Get the cache directory path
     */
    static std::string getCacheDirectory();

    /**
     * Get the state directory path
     */
    static std::string getStateDirectory();

    /**
     * Find the configuration file to use (follows XDG spec)
     */
    static std::string findConfigFile();

    /**
     * Get the database file path
     */
    static std::string getDatabasePath();

    /**
     * Get the mixes directory path (user's downloaded music)
     */
    static std::string getMixesDirectory();

    /**
     * Get the file mappings path (maps hash IDs to title-based filenames)
     */
    static std::string getFileMappingsPath();

    /**
     * Get the presets directory path
     */
    static std::string getPresetsDirectory();

    /**
     * Get the textures directory path
     */
    static std::string getTexturesDirectory();

    /**
     * Expand tilde in paths (cross-platform)
     */
    static std::string expandTilde(const std::string& path);

    /**
     * Get all possible config file locations in search order
     */
    static std::vector<std::string> getConfigFileSearchPaths();

    /**
     * Get all possible preset directories in search order
     */
    static std::vector<std::string> getPresetSearchPaths();

    /**
     * Get all possible texture directories in search order
     */
    static std::vector<std::string> getTextureSearchPaths();

    /**
     * Check if a path exists and is accessible
     */
    static bool pathExists(const std::string& path);

    /**
     * Create directory if it doesn't exist
     */
    static void ensureDirectoryExists(const std::string& path);

    /**
     * Get the application name (for directory naming)
     */
    static std::string getAppName();

    static bool isLinux();
    static bool isMacOS();
    static bool isWindows();

private:
    static std::string getXDGConfigHome();
    static std::string getXDGDataHome();
    static std::string getXDGCacheHome();
    static std::string getXDGStateHome();
    static std::vector<std::string> getXDGDataDirectories();
    static std::string getWindowsAppData();
    static std::string getMacOSAppSupport();
    static std::string getMacOSCaches();
    static std::string getWindowsProgramData();
    static std::string getWindowsProgramFiles();
    static std::string getHomeDirectory();
    static std::string joinPath(const std::string& base, const std::string& component);
    static std::string normalizePath(const std::string& path);
};