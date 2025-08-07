#pragma once

#include <chrono>
#include <string>

namespace AutoVibez {
namespace Utils {

/**
 * @brief Utility functions for datetime operations
 */
class DateTimeUtils {
public:
    /**
     * @brief Get current date and time as formatted string
     * @return Current datetime in "YYYY-MM-DD HH:MM:SS" format
     */
    static std::string getCurrentDateTime();

    /**
     * @brief Format a time point as a datetime string
     * @param time The time point to format
     * @return Formatted datetime string
     */
    static std::string formatDateTime(const std::chrono::system_clock::time_point& time);

    /**
     * @brief Get current date as formatted string
     * @return Current date in "YYYY-MM-DD" format
     */
    static std::string getCurrentDate();

    /**
     * @brief Get current time as formatted string
     * @return Current time in "HH:MM:SS" format
     */
    static std::string getCurrentTime();

    /**
     * @brief Parse a datetime string into a time point
     * @param datetime_str The datetime string to parse
     * @return Time point, or current time if parsing fails
     */
    static std::chrono::system_clock::time_point parseDateTime(const std::string& datetime_str);

    /**
     * @brief Check if a string is a valid datetime format
     * @param datetime_str The string to validate
     * @return True if valid datetime format, false otherwise
     */
    static bool isValidDateTime(const std::string& datetime_str);

private:
};

}  // namespace Utils
}  // namespace AutoVibez
