#pragma once

#include <fstream>
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

    /**
     * @brief Check if the file contains valid MP3 frames
     * @param file Open file stream to check
     * @param start_offset Offset to start checking from
     * @return True if valid MP3 frames found
     */
    static bool hasValidMP3Frames(std::ifstream& file, size_t start_offset);

    /**
     * @brief Validate MP3 frame header
     * @param data Pointer to frame header data
     * @param available_bytes Number of bytes available
     * @return True if frame header is valid
     */
    static bool isValidMP3FrameHeader(const char* data, size_t available_bytes);
};

}  // namespace Utils
}  // namespace AutoVibez
