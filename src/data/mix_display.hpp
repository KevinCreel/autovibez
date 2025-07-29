#ifndef MIX_DISPLAY_HPP
#define MIX_DISPLAY_HPP

#include "mix_metadata.hpp"
#include <string>

namespace AutoVibez {
namespace Data {

/**
 * @brief Handles terminal UI for mix information display
 */
class MixDisplay {
public:
    MixDisplay();
    ~MixDisplay();
    
    /**
     * @brief Display mix information
     * @param mix Mix to display
     */
    void displayMixInfo(const Mix& mix);
    
    /**
     * @brief Display playback status
     * @param mix Current mix
     * @param position Current position in seconds
     * @param duration Total duration in seconds
     * @param volume Current volume
     */
    void displayPlaybackStatus(const Mix& mix, int position, int duration, int volume);
    
    /**
     * @brief Display download progress
     * @param mix Mix being downloaded
     * @param progress Progress percentage (0-100)
     * @param downloaded_bytes Bytes downloaded
     * @param total_bytes Total bytes
     */
    void displayDownloadProgress(const Mix& mix, int progress, size_t downloaded_bytes, size_t total_bytes);
    
    /**
     * @brief Display cache status
     * @param used_mb Used cache in MB
     * @param total_mb Total cache in MB
     * @param mix_count Number of mixes in cache
     */
    void displayCacheStatus(size_t used_mb, size_t total_mb, int mix_count);

private:
    /**
     * @brief Format time in MM:SS format
     * @param seconds Time in seconds
     * @return Formatted time string
     */
    std::string formatTime(int seconds);
    
    /**
     * @brief Format file size in human readable format
     * @param bytes Size in bytes
     * @return Formatted size string
     */
    std::string formatFileSize(size_t bytes);
};

} // namespace Data
} // namespace AutoVibez

#endif // MIX_DISPLAY_HPP 