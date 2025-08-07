#ifndef AUDIO_UTILS_HPP
#define AUDIO_UTILS_HPP

#include <string>

namespace AutoVibez {
namespace Utils {

/**
 * @brief Utility functions for audio file operations
 */
class AudioUtils {
public:
    /**
     * @brief Check if a file is a valid MP3
     * @param file_path Path to the file to check
     * @return True if valid MP3, false otherwise
     */
    static bool isValidMP3File(const std::string& file_path);
    


private:
    /**
     * @brief Check if a file exists and is readable
     * @param file_path Path to the file
     * @return True if file exists and is readable
     */
    static bool fileExists(const std::string& file_path);
};

} // namespace Utils
} // namespace AutoVibez

#endif // AUDIO_UTILS_HPP
