#ifndef input_validator_hpp
#define input_validator_hpp

#include <string>
#include <vector>
#include <regex>

/**
 * @brief Input validation utilities for security and robustness
 */
class InputValidator {
public:
    /**
     * @brief Validate if a string is a valid file path
     * @param path The path to validate
     * @return true if valid, false otherwise
     */
    static bool isValidFilePath(const std::string& path);
    
    /**
     * @brief Validate if a string is a valid URL
     * @param url The URL to validate
     * @return true if valid, false otherwise
     */
    static bool isValidUrl(const std::string& url);
    
    /**
     * @brief Validate if a string is a valid audio file extension
     * @param filename The filename to validate
     * @return true if valid audio file, false otherwise
     */
    static bool isValidAudioFile(const std::string& filename);
    
    /**
     * @brief Sanitize input string to prevent injection attacks
     * @param input The input string to sanitize
     * @return Sanitized string
     */
    static std::string sanitizeInput(const std::string& input);
    
    /**
     * @brief Validate if a string contains only safe characters
     * @param input The input string to validate
     * @return true if safe, false otherwise
     */
    static bool containsOnlySafeChars(const std::string& input);
    
    /**
     * @brief Validate if a string is a valid integer
     * @param input The input string to validate
     * @return true if valid integer, false otherwise
     */
    static bool isValidInteger(const std::string& input);
    
    /**
     * @brief Validate if a string is a valid floating point number
     * @param input The input string to validate
     * @return true if valid float, false otherwise
     */
    static bool isValidFloat(const std::string& input);
    
    /**
     * @brief Validate if a string is within length limits
     * @param input The input string to validate
     * @param maxLength Maximum allowed length
     * @return true if within limits, false otherwise
     */
    static bool isValidLength(const std::string& input, size_t maxLength);

private:
    // Disable instantiation
    InputValidator() = delete;
    
    // Valid audio file extensions
    static const std::vector<std::string> VALID_AUDIO_EXTENSIONS;
    
    // Safe character pattern
    static const std::regex SAFE_CHAR_PATTERN;
    
    // URL pattern
    static const std::regex URL_PATTERN;
    
    // Integer pattern
    static const std::regex INTEGER_PATTERN;
    
    // Float pattern
    static const std::regex FLOAT_PATTERN;
};

#endif /* input_validator_hpp */ 