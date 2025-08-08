#pragma once

#include <curl/curl.h>

#include <mutex>
#include <string>

#include "error_handler.hpp"
#include "mix_metadata.hpp"

namespace AutoVibez {
namespace Audio {
class MP3Analyzer;
}
}  // namespace AutoVibez

namespace AutoVibez {
namespace Data {

/**
 * @brief RAII wrapper for FILE* handles
 */
class FileHandle {
public:
    FileHandle(const std::string& path, const std::string& mode);
    ~FileHandle();

    FILE* get() const {
        return file_;
    }
    bool isValid() const {
        return file_ != nullptr;
    }

    // Disable copying
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    // Allow moving
    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;

private:
    FILE* file_;
};

/**
 * @brief Handles downloading of mix files from URLs
 */
class MixDownloader : public AutoVibez::Utils::ErrorHandler {
public:
    MixDownloader(const std::string& mixes_dir);
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
     * @brief Get local path for a mix using original filename
     * @param mix Mix object containing original filename
     * @return Local file path
     */
    std::string getLocalPathWithOriginalFilename(const Mix& mix);

    /**
     * @brief Download a mix to a temporary file and rename based on MP3 title
     * @param mix Mix to download
     * @param mp3_analyzer MP3Analyzer instance to extract title
     * @return True if successful, false otherwise
     */
    bool downloadMixWithTitleNaming(const Mix& mix, AutoVibez::Audio::MP3Analyzer* mp3_analyzer);

    /**
     * @brief Get temporary path for a mix during download
     * @param mix_id Mix ID
     * @return Temporary file path
     */
    std::string getTemporaryPath(const std::string& mix_id);

    /**
     * @brief Get the last error message
     * @return Error message string
     */
    std::string getLastError() const;

    /**
     * @brief Validate mix ID format
     * @param mix_id Mix ID to validate
     * @return True if valid, false otherwise
     */
    static bool isValidMixId(const std::string& mix_id);

private:
    /**
     * @brief Initialize CURL with proper error handling
     * @return True if successful, false otherwise
     */
    bool initializeCurl();

    /**
     * @brief Cleanup CURL resources
     */
    void cleanupCurl();

    /**
     * @brief Download file using CURL with proper resource management
     * @param url URL to download from
     * @param file_path Local file path to save to
     * @return True if successful, false otherwise
     */
    bool downloadFileWithCurl(const std::string& url, const std::string& file_path);

    /**
     * @brief Copy local file with proper error handling
     * @param source_path Source file path
     * @param dest_path Destination file path
     * @return True if successful, false otherwise
     */
    bool copyLocalFile(const std::string& source_path, const std::string& dest_path);

    std::string mixes_dir;
    mutable std::mutex mutex_;  // For thread safety
};

}  // namespace Data
}  // namespace AutoVibez