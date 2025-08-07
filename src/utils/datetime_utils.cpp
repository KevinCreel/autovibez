#include "datetime_utils.hpp"

#include <iomanip>
#include <regex>
#include <sstream>

#include "constants.hpp"

namespace AutoVibez {
namespace Utils {

std::string DateTimeUtils::getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    return formatDateTime(now);
}

std::string DateTimeUtils::formatDateTime(const std::chrono::system_clock::time_point& time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    auto tm = *std::localtime(&time_t);

    std::stringstream ss;
    ss << std::put_time(&tm, StringConstants::DATETIME_FORMAT);
    return ss.str();
}

std::string DateTimeUtils::getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);

    std::stringstream ss;
    ss << std::put_time(&tm, StringConstants::DATE_FORMAT);
    return ss.str();
}

std::string DateTimeUtils::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);

    std::stringstream ss;
    ss << std::put_time(&tm, StringConstants::TIME_FORMAT);
    return ss.str();
}

std::chrono::system_clock::time_point DateTimeUtils::parseDateTime(const std::string& datetime_str) {
    if (!isValidDateTime(datetime_str)) {
        return std::chrono::system_clock::now();
    }

    std::tm tm = {};
    std::istringstream ss(datetime_str);
    ss >> std::get_time(&tm, StringConstants::DATETIME_FORMAT);

    if (ss.fail()) {
        return std::chrono::system_clock::now();
    }

    auto time_t = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time_t);
}

bool DateTimeUtils::isValidDateTime(const std::string& datetime_str) {
    // Check if string matches "YYYY-MM-DD HH:MM:SS" format
    std::regex datetime_regex(StringConstants::DATETIME_REGEX_PATTERN);
    return std::regex_match(datetime_str, datetime_regex);
}

std::string DateTimeUtils::getDefaultFormat() {
    return StringConstants::DATETIME_FORMAT;
}

}  // namespace Utils
}  // namespace AutoVibez
