#pragma once

#include "mix_metadata.hpp"
#include <string>

/**
 * @brief Handles downloading of mix files from URLs
 */
class MixDownloader {
public:
    explicit MixDownloader(const std::string& cache_dir);
    ~MixDownloader();
    
    /**
     * @brief Download a mix file from URL
     * @param mix Mix to download
     * @return True if successful, false otherwise
     */
    bool downloadMix(const Mix& mix);
    
    /**
     * @brief Check if a mix is already downloaded
     * @param mix_id Mix ID
     * @return True if downloaded, false otherwise
     */
    bool isMixDownloaded(const std::string& mix_id);
    
    /**
     * @brief Get local path for a mix
     * @param mix_id Mix ID
     * @return Local file path
     */
    std::string getLocalPath(const std::string& mix_id);
    
    /**
     * @brief Get the last error message
     * @return Error message string
     */
    std::string getLastError() const { return last_error; }

private:
    std::string cache_dir;
    std::string last_error;
}; 