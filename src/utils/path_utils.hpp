#pragma once

#include <filesystem>
#include <string>

namespace AutoVibez::Utils {

/**
 * @brief Utility class for common file path operations
 */
class PathUtils {
public:
    /**
     * @brief Join path components with proper directory separator
     * @param base Base path
     * @param component Component to join
     * @return Joined path
     */
    static std::string joinPath(const std::string& base, const std::string& component);

    /**
     * @brief Get file extension from path
     * @param file_path File path
     * @return File extension (without dot)
     */
    static std::string getFileExtension(const std::string& file_path);

    /**
     * @brief Get filename without extension from path
     * @param file_path File path
     * @return Filename without extension
     */
    static std::string getFilenameWithoutExtension(const std::string& file_path);

    /**
     * @brief Get filename with extension from path
     * @param file_path File path
     * @return Filename with extension
     */
    static std::string getFilename(const std::string& file_path);

    /**
     * @brief Create a safe filename by replacing invalid characters
     * @param title Original title
     * @return Safe filename
     */
    static std::string createSafeFilename(const std::string& title);

    /**
     * @brief Check if file has specific extension
     * @param file_path File path
     * @param extension Extension to check (without dot)
     * @return True if file has the specified extension
     */
    static bool hasExtension(const std::string& file_path, const std::string& extension);
};

}  // namespace AutoVibez::Utils
