#include "audio_utils.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

#include "constants.hpp"
#include "path_utils.hpp"

namespace AutoVibez {
namespace Utils {

bool AudioUtils::isValidMP3File(const std::string& file_path) {
    if (!fileExists(file_path)) {
        return false;
    }

    if (!AutoVibez::Utils::PathUtils::hasExtension(file_path, "mp3")) {
        return false;
    }

    if (std::filesystem::file_size(file_path) < Constants::MIN_MP3_FILE_SIZE) {
        return false;
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Check for ID3v2 header first
    char buffer[Constants::ID3V2_HEADER_SIZE];
    file.read(buffer, Constants::ID3V2_HEADER_SIZE);

    if (file.gcount() < Constants::ID3V2_HEADER_SIZE) {
        file.close();
        return false;
    }

    size_t offset = 0;

    // Check for ID3v2 header
    if (buffer[0] == 'I' && buffer[1] == 'D' && buffer[2] == '3') {
        // ID3v2 header found, skip to MP3 data
        // ID3v2 header is 10 bytes, then we need to skip the tag data
        offset = Constants::ID3V2_HEADER_SIZE;

        // Read tag size (4 bytes after header)
        unsigned char tag_size_bytes[4];
        file.seekg(6);  // Skip to tag size bytes
        file.read(reinterpret_cast<char*>(tag_size_bytes), 4);

        if (file.gcount() == 4) {
            // Calculate tag size (synchsafe integer)
            size_t tag_size =
                (tag_size_bytes[0] << 21) | (tag_size_bytes[1] << 14) | (tag_size_bytes[2] << 7) | tag_size_bytes[3];

            offset += tag_size;
        }
    }

    // Now check for valid MP3 frame structure
    if (!hasValidMP3Frames(file, offset)) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool AudioUtils::hasValidMP3Frames(std::ifstream& file, size_t start_offset) {
    file.seekg(start_offset);

    // Read a larger buffer to check multiple frames
    const size_t buffer_size = 4096;
    char buffer[buffer_size];

    file.read(buffer, buffer_size);
    size_t bytes_read = file.gcount();

    if (bytes_read < 4) {
        return false;
    }

    // Look for MPEG sync bytes (0xFF followed by 0xE0-0xFF)
    for (size_t i = 0; i < bytes_read - 3; i++) {
        if (static_cast<unsigned char>(buffer[i]) == 0xFF) {
            unsigned char second_byte = static_cast<unsigned char>(buffer[i + 1]);

            // Check if this looks like a valid MPEG sync
            if ((second_byte & 0xE0) == 0xE0) {
                // Found potential MPEG sync, validate frame header
                if (isValidMP3FrameHeader(buffer + i, bytes_read - i)) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool AudioUtils::isValidMP3FrameHeader(const char* data, size_t available_bytes) {
    if (available_bytes < 4) {
        return false;
    }

    // MP3 frame header is 4 bytes
    unsigned char header[4];
    for (int i = 0; i < 4; i++) {
        header[i] = static_cast<unsigned char>(data[i]);
    }

    // Validate MPEG sync (first 11 bits should be 0xFF)
    if ((header[0] != 0xFF) || ((header[1] & 0xE0) != 0xE0)) {
        return false;
    }

    // Extract and validate MPEG version and layer
    unsigned char mpeg_version = (header[1] >> 3) & 0x03;
    unsigned char layer = (header[1] >> 1) & 0x03;

    // Valid MPEG versions: 0 (MPEG-2.5), 2 (MPEG-2), 3 (MPEG-1)
    if (mpeg_version == 1) {
        return false;  // Reserved value
    }

    // Valid layers: 1 (Layer III), 2 (Layer II), 3 (Layer I)
    if (layer == 0) {
        return false;  // Reserved value
    }

    // Extract bitrate and sample rate to validate they're reasonable
    unsigned char bitrate_index = (header[2] >> 4) & 0x0F;
    unsigned char sample_rate_index = (header[2] >> 2) & 0x03;

    // Check for valid bitrate index (0 is "free" format, 15 is invalid)
    if (bitrate_index == 15) {
        return false;
    }

    // Check for valid sample rate index
    if (sample_rate_index == 3) {
        return false;
    }

    return true;
}

bool AudioUtils::fileExists(const std::string& file_path) {
    return std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path);
}

}  // namespace Utils
}  // namespace AutoVibez
