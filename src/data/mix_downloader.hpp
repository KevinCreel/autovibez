#pragma once

#include <curl/curl.h>

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
    std::string getLastError() const {
        return last_error;
    }

private:
    std::string mixes_dir;
};

}  // namespace Data
}  // namespace AutoVibez