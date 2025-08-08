#include "json_utils.hpp"

#include <sstream>

namespace AutoVibez::Utils {

std::string JsonUtils::vectorToJsonArray(const std::vector<std::string>& tags) {
    if (tags.empty()) {
        return "[]";
    }

    std::ostringstream oss;
    oss << "[";

    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) {
            oss << ",";
        }
        oss << "\"" << escapeJsonString(tags[i]) << "\"";
    }

    oss << "]";
    return oss.str();
}

std::vector<std::string> JsonUtils::jsonArrayToVector(const std::string& json_array) {
    std::vector<std::string> result;

    if (json_array.empty() || json_array == "[]") {
        return result;
    }

    // Simple parser for JSON array format: ["tag1","tag2","tag3"]
    std::string current_tag;
    bool in_string = false;
    bool escaped = false;

    for (size_t i = 0; i < json_array.length(); ++i) {
        char c = json_array[i];

        if (escaped) {
            current_tag += c;
            escaped = false;
            continue;
        }

        if (c == '\\' && in_string) {
            escaped = true;
            continue;
        }

        if (c == '"') {
            if (in_string) {
                // End of string
                result.push_back(unescapeJsonString(current_tag));
                current_tag.clear();
                in_string = false;
            } else {
                // Start of string
                in_string = true;
            }
        } else if (in_string) {
            current_tag += c;
        }
        // Ignore other characters (brackets, commas, spaces) when not in string
    }

    return result;
}

std::string JsonUtils::escapeJsonString(const std::string& str) {
    std::string result;
    result.reserve(str.length() * 2);  // Reserve space for potential escapes

    for (char c : str) {
        switch (c) {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
                break;
        }
    }

    return result;
}

std::string JsonUtils::unescapeJsonString(const std::string& str) {
    std::string result;
    result.reserve(str.length());

    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"':
                    result += '"';
                    ++i;
                    break;
                case '\\':
                    result += '\\';
                    ++i;
                    break;
                case 'b':
                    result += '\b';
                    ++i;
                    break;
                case 'f':
                    result += '\f';
                    ++i;
                    break;
                case 'n':
                    result += '\n';
                    ++i;
                    break;
                case 'r':
                    result += '\r';
                    ++i;
                    break;
                case 't':
                    result += '\t';
                    ++i;
                    break;
                default:
                    result += str[i];
                    break;
            }
        } else {
            result += str[i];
        }
    }

    return result;
}

}  // namespace AutoVibez::Utils
