#pragma once

#include <string>
#include <filesystem>

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
     * Find the configuration file to use
     */
    static std::string findConfigFile();
    
    /**
     * Expand tilde in paths (cross-platform)
     */
    static std::string expandTilde(const std::string& path);
    
    /**
     * Get the cache directory path
     */
    static std::string getCacheDirectory();
    
    /**
     * Get the state directory path
     */
    static std::string getStateDirectory();
    
private:
    /**
     * Get XDG config home directory
     */
    static std::string getXDGConfigHome();
    
    /**
     * Get XDG data home directory
     */
    static std::string getXDGDataHome();
    
    /**
     * Create directory if it doesn't exist
     */
    static void ensureDirectoryExists(const std::string& path);
}; 