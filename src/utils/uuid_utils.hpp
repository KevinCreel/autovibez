#pragma once

#include <string>

namespace AutoVibez {
namespace Utils {

/**
 * @brief Utility functions for UUID generation
 */
class UuidUtils {
public:
    /**
     * @brief Generate a deterministic UUID v5 from a URL
     * @param url The URL to generate UUID from
     * @return Deterministic UUID v5 string
     */
    static std::string generateIdFromUrl(const std::string& url);

private:
    /**
     * @brief Generate a deterministic UUID v5 from a hash
     * @param hash The hash value to generate UUID from
     * @return Deterministic UUID v5 string
     */
    static std::string generateUuidFromHash(size_t hash);
};

}  // namespace Utils
}  // namespace AutoVibez
