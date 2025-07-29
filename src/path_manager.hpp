#pragma once

#include <string>
#include <filesystem>
#include <vector>

/**
 * Single source of truth for all directory and path management.
 * Provides XDG-compliant, cross-platform directory resolution.
 */
class PathManager {
public:
    // ===== Core Directory Methods =====
    
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
    
    // ===== File Path Methods =====
    
    /**
     * Find the configuration file to use (follows XDG spec)
     */
    static std::string findConfigFile();
    
    /**
     * Get the database file path
     */
    static std::string getDatabasePath();
    
    /**
     * Get the mix cache directory path
     */
    static std::string getMixCacheDirectory();
    
    /**
     * Get the presets directory path
     */
    static std::string getPresetsDirectory();
    
    /**
     * Get the textures directory path
     */
    static std::string getTexturesDirectory();
    
    // ===== Utility Methods =====
    
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
    
    // ===== Platform Detection =====
    
    /**
     * Check if running on Linux/Unix
     */
    static bool isLinux();
    
    /**
     * Check if running on macOS
     */
    static bool isMacOS();
    
    /**
     * Check if running on Windows
     */
    static bool isWindows();
    
private:
    // ===== XDG Base Directory Methods =====
    
    /**
     * Get XDG config home directory
     */
    static std::string getXDGConfigHome();
    
    /**
     * Get XDG data home directory
     */
    static std::string getXDGDataHome();
    
    /**
     * Get XDG cache home directory
     */
    static std::string getXDGCacheHome();
    
    /**
     * Get XDG state home directory
     */
    static std::string getXDGStateHome();
    
    /**
     * Get XDG data directories (for fallback search)
     */
    static std::vector<std::string> getXDGDataDirectories();
    
    // ===== Platform-Specific Methods =====
    
    /**
     * Get Windows app data directory
     */
    static std::string getWindowsAppData();
    
    /**
     * Get macOS application support directory
     */
    static std::string getMacOSAppSupport();
    
    /**
     * Get macOS caches directory
     */
    static std::string getMacOSCaches();
    
    // ===== Internal Utilities =====
    
    /**
     * Get home directory (cross-platform)
     */
    static std::string getHomeDirectory();
    
    /**
     * Join path components (cross-platform)
     */
    static std::string joinPath(const std::string& base, const std::string& component);
    
    /**
     * Normalize path separators for current platform
     */
    static std::string normalizePath(const std::string& path);
}; 