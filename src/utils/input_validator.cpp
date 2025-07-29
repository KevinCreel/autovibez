#include "input_validator.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>

// Static member initialization
const std::vector<std::string> InputValidator::VALID_AUDIO_EXTENSIONS = {
    ".mp3", ".wav", ".flac", ".ogg", ".m4a", ".aac", ".wma"
};

const std::regex InputValidator::SAFE_CHAR_PATTERN(R"([a-zA-Z0-9\s\-_\.\/\\:]+)");
const std::regex InputValidator::URL_PATTERN(R"(https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*))");
const std::regex InputValidator::INTEGER_PATTERN(R"(^[+-]?\d+$)");
const std::regex InputValidator::FLOAT_PATTERN(R"(^[+-]?\d*\.?\d+$)");

// Constants
const size_t MAX_PATH_LENGTH = 4096;

bool InputValidator::isValidFilePath(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    // Check for null bytes or other dangerous characters
    if (path.find('\0') != std::string::npos) {
        return false;
    }
    
    // Check for path traversal attempts
    if (path.find("..") != std::string::npos) {
        return false;
    }
    
    // Check length limit
    if (path.length() > MAX_PATH_LENGTH) {
        return false;
    }
    
    // Check for safe characters only
    return std::regex_match(path, SAFE_CHAR_PATTERN);
}

bool InputValidator::isValidUrl(const std::string& url) {
    if (url.empty()) {
        return false;
    }
    
    // Check length limits
    if (!isValidLength(url, 2048)) {
        return false;
    }
    
    // Use regex to validate URL format
    return std::regex_match(url, URL_PATTERN);
}

bool InputValidator::isValidAudioFile(const std::string& filename) {
    if (filename.empty()) {
        return false;
    }
    
    // Convert to lowercase for comparison
    std::string lowerFilename = filename;
    std::transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);
    
    // Check if it has a valid audio extension
    for (const auto& ext : VALID_AUDIO_EXTENSIONS) {
        if (lowerFilename.length() >= ext.length() && 
            lowerFilename.substr(lowerFilename.length() - ext.length()) == ext) {
            // Additional check: filename should not be just the extension
            if (lowerFilename == ext) {
                return false;
            }
            return true;
        }
    }
    
    return false;
}

std::string InputValidator::sanitizeInput(const std::string& input) {
    std::string sanitized;
    sanitized.reserve(input.length());
    
    for (char c : input) {
        // Remove null bytes
        if (c == '\0') {
            continue;
        }
        
        // Remove control characters except newline and tab
        if (c < 32 && c != '\n' && c != '\t') {
            continue;
        }
        
        sanitized += c;
    }
    
    // Trim whitespace
    sanitized.erase(0, sanitized.find_first_not_of(" \t\n\r"));
    sanitized.erase(sanitized.find_last_not_of(" \t\n\r") + 1);
    
    return sanitized;
}

bool InputValidator::containsOnlySafeChars(const std::string& input) {
    if (input.empty()) {
        return true;
    }
    
    // Check for null bytes first
    if (input.find('\0') != std::string::npos) {
        return false;
    }
    
    return std::regex_match(input, SAFE_CHAR_PATTERN);
}

bool InputValidator::isValidInteger(const std::string& input) {
    if (input.empty()) {
        return false;
    }
    
    return std::regex_match(input, INTEGER_PATTERN);
}

bool InputValidator::isValidFloat(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    
    // Use a more flexible regex for float validation
    std::regex float_pattern(R"(^[+-]?\d*\.?\d+$|^[+-]?\d+\.?$)");
    return std::regex_match(str, float_pattern);
}

bool InputValidator::isValidLength(const std::string& input, size_t maxLength) {
    return input.length() <= maxLength;
} 