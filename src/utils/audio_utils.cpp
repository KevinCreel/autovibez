#include "audio_utils.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>

namespace AutoVibez {
namespace Utils {

bool AudioUtils::isValidMP3File(const std::string& file_path) {
    // Check if file exists
    if (!fileExists(file_path)) {
        return false;
    }
    
    // Check file extension
    std::filesystem::path path(file_path);
    std::string extension = path.extension().string();
    
    // Remove the dot and convert to lowercase
    if (!extension.empty() && extension[0] == '.') {
        extension = extension.substr(1);
    }
    
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (extension != "mp3") {
        return false;
    }
    
    // Check file size (must be at least 1KB to be a valid MP3)
    if (std::filesystem::file_size(path) < 1024) {
        return false;
    }
    
    // Check for MP3 file signature (ID3v2 or MPEG sync)
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    char buffer[10];
    file.read(buffer, 10);
    file.close();
    
    if (file.gcount() < 10) {
        return false;
    }
    
    // Check for ID3v2 tag (starts with "ID3")
    if (buffer[0] == 'I' && buffer[1] == 'D' && buffer[2] == '3') {
        return true;
    }
    
    // Check for MPEG sync (0xFF followed by 0xE0-0xFF)
    if ((static_cast<unsigned char>(buffer[0]) == 0xFF) && 
        ((static_cast<unsigned char>(buffer[1]) & 0xE0) == 0xE0)) {
        return true;
    }
    
    return false;
}



bool AudioUtils::fileExists(const std::string& file_path) {
    return std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path);
}

} // namespace Utils
} // namespace AutoVibez
