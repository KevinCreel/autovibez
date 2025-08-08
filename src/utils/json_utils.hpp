#pragma once

#include <string>
#include <vector>

namespace AutoVibez {
namespace Utils {

/**
 * @brief Simple JSON utility for handling tag arrays
 *
 * This is a lightweight JSON parser specifically for handling the simple
 * tag array format used in the database. For more complex JSON parsing,
 * consider using a dedicated JSON library like nlohmann/json.
 */
class JsonUtils {
public:
    /**
     * @brief Convert vector of strings to JSON array format
     * @param tags Vector of tag strings
     * @return JSON array string (e.g., ["tag1","tag2"])
     */
    static std::string vectorToJsonArray(const std::vector<std::string>& tags);

    /**
     * @brief Parse JSON array string to vector of strings
     * @param json_array JSON array string
     * @return Vector of parsed strings
     */
    static std::vector<std::string> jsonArrayToVector(const std::string& json_array);

private:
    /**
     * @brief Escape special characters in JSON string
     * @param str String to escape
     * @return Escaped string
     */
    static std::string escapeJsonString(const std::string& str);

    /**
     * @brief Unescape JSON string
     * @param str Escaped string
     * @return Unescaped string
     */
    static std::string unescapeJsonString(const std::string& str);
};

}  // namespace Utils
}  // namespace AutoVibez
