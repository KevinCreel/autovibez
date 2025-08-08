#pragma once

#include <string>

namespace AutoVibez::Utils {

/**
 * @brief Utility functions for generating deterministic hash-based IDs
 */
class HashIdUtils {
public:
    /**
     * @brief Generate a deterministic hash-based ID from a URL
     * @param url The URL to generate ID from
     * @return Deterministic hash-based ID string in UUID format
     */
    static std::string generateIdFromUrl(const std::string& url);

private:
    /**
     * @brief Generate a deterministic hash-based ID from a hash value
     * @param hash The hash value to generate ID from
     * @return Deterministic hash-based ID string in UUID format
     */
    static std::string generateIdFromHash(size_t hash);
};

}  // namespace AutoVibez::Utils
