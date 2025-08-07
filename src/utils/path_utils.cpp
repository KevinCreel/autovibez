#include "path_utils.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

#include "constants.hpp"

namespace AutoVibez {
namespace Utils {

std::string PathUtils::joinPath(const std::string& base, const std::string& component) {
    std::filesystem::path base_path(base);
    std::filesystem::path component_path(component);
    return (base_path / component_path).string();
}

std::string PathUtils::getFileExtension(const std::string& file_path) {
    std::filesystem::path path(file_path);
    std::string extension = path.extension().string();

    // Remove the dot and convert to lowercase
    if (!extension.empty() && extension[0] == '.') {
        extension = extension.substr(1);
    }

    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return extension;
}

std::string PathUtils::getFilenameWithoutExtension(const std::string& file_path) {
    std::filesystem::path path(file_path);
    return path.stem().string();
}

std::string PathUtils::getFilename(const std::string& file_path) {
    std::filesystem::path path(file_path);
    return path.filename().string();
}

std::string PathUtils::createSafeFilename(const std::string& title) {
    std::string safe_filename = title;

    // Replace invalid characters with underscores
    const std::string invalid_chars = "<>:\"/\\|?*";
    size_t pos = 0;
    while ((pos = safe_filename.find_first_of(invalid_chars, pos)) != std::string::npos) {
        safe_filename[pos] = '_';
        pos++;
    }

    // Remove leading/trailing spaces and dots
    safe_filename.erase(0, safe_filename.find_first_not_of(" ."));
    safe_filename.erase(safe_filename.find_last_not_of(" .") + 1);

    // Remove trailing underscores
    while (!safe_filename.empty() && safe_filename.back() == '_') {
        safe_filename.pop_back();
    }

    // Limit length
    if (safe_filename.length() > 200) {
        safe_filename = safe_filename.substr(0, 200);
    }

    return safe_filename;
}

bool PathUtils::hasExtension(const std::string& file_path, const std::string& extension) {
    std::string file_extension = getFileExtension(file_path);
    std::string target_extension = extension;
    std::transform(target_extension.begin(), target_extension.end(), target_extension.begin(), ::tolower);
    return file_extension == target_extension;
}

}  // namespace Utils
}  // namespace AutoVibez
