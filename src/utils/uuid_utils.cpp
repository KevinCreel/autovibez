#include "uuid_utils.hpp"

#include <functional>
#include <iomanip>
#include <sstream>

#include "constants.hpp"

namespace AutoVibez {
namespace Utils {

std::string UuidUtils::generateIdFromUrl(const std::string& url) {
    // Generate a deterministic UUID v5 based on URL hash
    std::hash<std::string> hasher;
    size_t hash = hasher(url);
    return generateUuidFromHash(hash);
}

std::string UuidUtils::generateUuidFromHash(size_t hash) {
    // Use the hash to generate a deterministic UUID v5 (name-based)
    unsigned char uuid_bytes[Constants::UUID_BYTE_LENGTH];

    // Use first 16 bytes of hash (or repeat if shorter)
    for (int i = 0; i < Constants::UUID_BYTE_LENGTH; i++) {
        uuid_bytes[i] = (hash >> (i % 8 * 8)) & 0xFF;
    }

    // Set version (5) and variant bits for deterministic UUID
    uuid_bytes[6] = (uuid_bytes[6] & 0x0F) | 0x50;  // Version 5
    uuid_bytes[8] = (uuid_bytes[8] & 0x3F) | 0x80;  // Variant 1

    // Convert to UUID string format
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < Constants::UUID_BYTE_LENGTH; i++) {
        if (i == Constants::UUID_POSITION_1 || i == Constants::UUID_POSITION_2 || i == Constants::UUID_POSITION_3 ||
            i == Constants::UUID_POSITION_4) {
            ss << "-";
        }
        ss << std::setw(2) << static_cast<int>(uuid_bytes[i]);
    }

    return ss.str();
}

}  // namespace Utils
}  // namespace AutoVibez
