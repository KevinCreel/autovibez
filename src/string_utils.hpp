#pragma once

#include <string>
#include <algorithm>
#include <cctype>

namespace StringUtils {
    /**
     * Convert a string to Title Case (first letter of each word capitalized)
     */
    inline std::string toTitleCase(const std::string& input) {
        std::string result = input;
        bool capitalize = true;
        
        for (char& c : result) {
            if (capitalize && std::isalpha(c)) {
                c = std::toupper(c);
                capitalize = false;
            } else if (std::isspace(c) || c == '-') {
                capitalize = true;
            } else if (std::isalpha(c)) {
                c = std::tolower(c);
            }
        }
        
        return result;
    }
    
    /**
     * Extract filename from a path
     */
    inline std::string extractFileName(const std::string& path) {
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash != std::string::npos) {
            return path.substr(lastSlash + 1);
        }
        return path;
    }
    
    /**
     * Format preset name for display (extract filename from path)
     */
    inline std::string formatPresetName(const std::string& presetPath) {
        return extractFileName(presetPath);
    }
    
    /**
     * Trim whitespace from both ends of a string
     */
    inline std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string::npos) return "";
        
        size_t end = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(start, end - start + 1);
    }
    
    /**
     * Check if a string starts with a given prefix
     */
    inline bool startsWith(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() && 
               str.compare(0, prefix.size(), prefix) == 0;
    }
    
    /**
     * Check if a string ends with a given suffix
     */
    inline bool endsWith(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() && 
               str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
} 