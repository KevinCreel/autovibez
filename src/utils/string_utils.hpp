#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace AutoVibez {
namespace Utils {

/**
 * @brief Utility class for common string processing operations
 */
class StringUtils {
public:
    /**
     * @brief Trim whitespace from both ends of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);

    /**
     * @brief Convert string to lowercase
     * @param str String to convert
     * @return Lowercase string
     */
    static std::string toLower(const std::string& str);

    /**
     * @brief Convert string to uppercase
     * @param str String to convert
     * @return Uppercase string
     */
    static std::string toUpper(const std::string& str);

    /**
     * @brief Replace all occurrences of a character in a string
     * @param str String to process
     * @param old_char Character to replace
     * @param new_char Character to replace with
     * @return String with replacements
     */
    static std::string replaceChar(const std::string& str, char old_char, char new_char);

    /**
     * @brief Replace all occurrences of characters from a set
     * @param str String to process
     * @param chars_to_replace Characters to replace
     * @param replacement_char Character to replace with
     * @return String with replacements
     */
    static std::string replaceChars(const std::string& str, const std::string& chars_to_replace, char replacement_char);

    /**
     * @brief Check if string starts with a prefix
     * @param str String to check
     * @param prefix Prefix to look for
     * @return True if string starts with prefix
     */
    static bool startsWith(const std::string& str, const std::string& prefix);

    /**
     * @brief Check if string ends with a suffix
     * @param str String to check
     * @param suffix Suffix to look for
     * @return True if string ends with suffix
     */
    static bool endsWith(const std::string& str, const std::string& suffix);

    /**
     * @brief Get the last occurrence of a character in a string
     * @param str String to search
     * @param ch Character to find
     * @return Position of last occurrence, or std::string::npos if not found
     */
    static size_t findLastOf(const std::string& str, char ch);

    /**
     * @brief Get the first occurrence of a character in a string
     * @param str String to search
     * @param ch Character to find
     * @return Position of first occurrence, or std::string::npos if not found
     */
    static size_t findFirstOf(const std::string& str, char ch);
};

}  // namespace Utils
}  // namespace AutoVibez
