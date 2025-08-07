#include "url_utils.hpp"

#include <algorithm>
#include <cctype>
#include <regex>

#include "constants.hpp"
#include "string_utils.hpp"

namespace AutoVibez {
namespace Utils {

std::string UrlUtils::extractFilenameFromUrl(const std::string& url) {
    if (url.empty()) {
        return "";
    }

    // Find the last '/' character
    size_t last_slash = url.find_last_of('/');
    if (last_slash == std::string::npos) {
        return "";
    }

    // Extract the filename part after the last '/'
    std::string filename = url.substr(last_slash + 1);

    // Remove query parameters and fragments
    size_t query_start = filename.find('?');
    if (query_start != std::string::npos) {
        filename = filename.substr(0, query_start);
    }

    size_t fragment_start = filename.find('#');
    if (fragment_start != std::string::npos) {
        filename = filename.substr(0, fragment_start);
    }

    // URL decode the filename
    return urlDecode(filename);
}

std::string UrlUtils::urlDecode(const std::string& encoded) {
    std::string result;
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            // Convert hex to char
            std::string hex = encoded.substr(i + 1, 2);
            if (isHexDigit(hex[0]) && isHexDigit(hex[1])) {
                char decoded_char = static_cast<char>(hexToInt(hex[0]) * Constants::HEX_BASE + hexToInt(hex[1]));
                result += decoded_char;
                i += 2;  // Skip the next two characters
            } else {
                result += encoded[i];
            }
        } else if (encoded[i] == '+') {
            result += ' ';
        } else {
            result += encoded[i];
        }
    }
    return result;
}

std::string UrlUtils::getUrlExtension(const std::string& url) {
    std::string filename = extractFilenameFromUrl(url);
    if (filename.empty()) {
        return "";
    }

    // Find the last '.' character
    size_t last_dot = filename.find_last_of('.');
    if (last_dot == std::string::npos) {
        return "";
    }

    // Extract the extension
    std::string extension = filename.substr(last_dot + 1);

    // Convert to lowercase
    extension = AutoVibez::Utils::StringUtils::toLower(extension);
    return extension;
}

bool UrlUtils::isValidUrl(const std::string& url) {
    if (url.empty()) {
        return false;
    }

    // Basic URL validation regex
    std::regex url_regex(StringConstants::URL_REGEX_PATTERN);
    return std::regex_match(url, url_regex);
}

std::string UrlUtils::getDomain(const std::string& url) {
    if (!isValidUrl(url)) {
        return "";
    }

    // Find the protocol separator
    size_t protocol_end = url.find("://");
    if (protocol_end == std::string::npos) {
        return "";
    }

    // Start after the protocol
    size_t domain_start = protocol_end + 3;

    // Find the first slash or query parameter
    size_t domain_end = url.find('/', domain_start);
    if (domain_end == std::string::npos) {
        domain_end = url.find('?', domain_start);
    }
    if (domain_end == std::string::npos) {
        domain_end = url.find('#', domain_start);
    }
    if (domain_end == std::string::npos) {
        domain_end = url.length();
    }

    return url.substr(domain_start, domain_end - domain_start);
}

std::string UrlUtils::getProtocol(const std::string& url) {
    if (url.empty()) {
        return "";
    }

    size_t protocol_end = url.find("://");
    if (protocol_end == std::string::npos) {
        return "";
    }

    return url.substr(0, protocol_end);
}

bool UrlUtils::isHexDigit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int UrlUtils::hexToInt(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + Constants::HEX_ALPHA_OFFSET;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + Constants::HEX_ALPHA_OFFSET;
    }
    return 0;
}

}  // namespace Utils
}  // namespace AutoVibez
