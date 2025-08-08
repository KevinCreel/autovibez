#include "string_utils.hpp"

#include <algorithm>
#include <cctype>

namespace AutoVibez::Utils {

std::string StringUtils::trim(const std::string& str) {
    static const char whitespace[] = " \n\t\v\r\f";
    std::string result = str;
    result.erase(0, result.find_first_not_of(whitespace));
    result.erase(result.find_last_not_of(whitespace) + 1);
    return result;
}

std::string StringUtils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string StringUtils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string StringUtils::replaceChar(const std::string& str, char old_char, char new_char) {
    std::string result = str;
    std::replace(result.begin(), result.end(), old_char, new_char);
    return result;
}

std::string StringUtils::replaceChars(const std::string& str, const std::string& chars_to_replace,
                                      char replacement_char) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find_first_of(chars_to_replace, pos)) != std::string::npos) {
        result[pos] = replacement_char;
        pos++;
    }
    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix) {
    if (prefix.length() > str.length()) {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

size_t StringUtils::findLastOf(const std::string& str, char ch) {
    return str.find_last_of(ch);
}

size_t StringUtils::findFirstOf(const std::string& str, char ch) {
    return str.find_first_of(ch);
}

}  // namespace AutoVibez::Utils
