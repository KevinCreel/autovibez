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
    // Check if file exists
    if (!fileExists(file_path)) {
        return false;
    }

    // Check file extension
    if (!AutoVibez::Utils::PathUtils::hasExtension(file_path, "mp3")) {
        return false;
    }

    // Check file size (must be at least 1KB to be a valid MP3)
    if (std::filesystem::file_size(file_path) < Constants::MIN_MP3_FILE_SIZE) {
        return false;
    }

    // Check for MP3 file signature (ID3v2 or MPEG sync)
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    char buffer[Constants::ID3V2_HEADER_SIZE];
    file.read(buffer, Constants::ID3V2_HEADER_SIZE);
    file.close();

    if (file.gcount() < Constants::ID3V2_HEADER_SIZE) {
        return false;
    }

    // Check for ID3v2 tag (starts with "ID3")
    if (buffer[0] == 'I' && buffer[1] == 'D' && buffer[2] == '3') {
        return true;
    }

    // Check for MPEG sync (0xFF followed by 0xE0-0xFF)
    if ((static_cast<unsigned char>(buffer[0]) == 0xFF) && ((static_cast<unsigned char>(buffer[1]) & 0xE0) == 0xE0)) {
        return true;
    }

    return false;
}

bool AudioUtils::fileExists(const std::string& file_path) {
    return std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path);
}

}  // namespace Utils
}  // namespace AutoVibez
