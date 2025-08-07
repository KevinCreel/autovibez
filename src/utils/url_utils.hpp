#pragma once

#include <string>

namespace AutoVibez {
namespace Utils {

/**
 * @brief Utility functions for URL operations
 */
class UrlUtils {
public:
    /**
     * @brief Extract filename from URL
     * @param url URL to extract filename from
     * @return Extracted filename or empty string if not found
     */
    static std::string extractFilenameFromUrl(const std::string& url);

    /**
     * @brief URL decode a string
     * @param encoded The encoded string
     * @return Decoded string
     */
    static std::string urlDecode(const std::string& encoded);

    /**
     * @brief Get file extension from URL
     * @param url URL to extract extension from
     * @return File extension (lowercase, without dot) or empty string
     */
    static std::string getUrlExtension(const std::string& url);

    /**
     * @brief Check if a string is a valid URL
     * @param url The string to validate
     * @return True if valid URL, false otherwise
     */
    static bool isValidUrl(const std::string& url);

    /**
     * @brief Get the domain from a URL
     * @param url The URL to extract domain from
     * @return Domain name or empty string if invalid
     */
    static std::string getDomain(const std::string& url);

    /**
     * @brief Get the protocol from a URL
     * @param url The URL to extract protocol from
     * @return Protocol (http, https, ftp, etc.) or empty string
     */
    static std::string getProtocol(const std::string& url);

private:
    /**
     * @brief Check if a character is a valid hex digit
     * @param c Character to check
     * @return True if valid hex digit, false otherwise
     */
    static bool isHexDigit(char c);

    /**
     * @brief Convert a hex character to its integer value
     * @param c Hex character (0-9, a-f, A-F)
     * @return Integer value (0-15)
     */
    static int hexToInt(char c);
};

}  // namespace Utils
}  // namespace AutoVibez
