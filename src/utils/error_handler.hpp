#ifndef ERROR_HANDLER_HPP
#define ERROR_HANDLER_HPP

#include <string>

namespace AutoVibez {
namespace Utils {

/**
 * @brief Base class providing common error handling functionality
 * 
 * This class provides a consistent error handling pattern that can be
 * inherited by or composed with other classes that need error handling.
 */
class ErrorHandler {
public:
    ErrorHandler() : success(true) {}
    virtual ~ErrorHandler() = default;
    
    /**
     * @brief Get the last error message
     * @return Error message string
     */
    std::string getLastError() const { return last_error; }
    
    /**
     * @brief Check if the last operation was successful
     * @return True if successful, false otherwise
     */
    bool isSuccess() const { return success; }
    
    /**
     * @brief Clear the current error state
     */
    void clearError() { 
        last_error.clear(); 
        success = true; 
    }

protected:
    /**
     * @brief Set an error message
     * @param error The error message to set
     */
    void setError(const std::string& error) {
        last_error = error;
        success = false;
    }
    
    /**
     * @brief Set success state
     * @param is_success Whether the operation was successful
     */
    void setSuccess(bool is_success) {
        success = is_success;
        if (is_success) {
            last_error.clear();
        }
    }

protected:
    std::string last_error;
    bool success;
};

} // namespace Utils
} // namespace AutoVibez

#endif // ERROR_HANDLER_HPP
