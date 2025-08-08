#include "uuid_utils.hpp"

#include <iomanip>
#include <sstream>

#include "constants.hpp"

namespace AutoVibez {
namespace Utils {

std::string HashIdUtils::generateIdFromUrl(const std::string& url) {
    // Generate a deterministic hash-based ID from URL
    std::hash<std::string> hasher;
    size_t hash = hasher(url);
    return generateIdFromHash(hash);
}

std::string HashIdUtils::generateIdFromHash(size_t hash) {
    // Use the hash to generate a deterministic hash-based ID in UUID format
    unsigned char id_bytes[Constants::UUID_BYTE_LENGTH];

    // Use first 16 bytes of hash (or repeat if shorter)
    for (int i = 0; i < Constants::UUID_BYTE_LENGTH; i++) {
        id_bytes[i] = (hash >> (i % 8 * 8)) & 0xFF;
    }

    // Set version (5) and variant bits for deterministic ID
    id_bytes[6] = (id_bytes[6] & Constants::UUID_VERSION_MASK) | Constants::UUID_VERSION_5;  // Version 5
    id_bytes[8] = (id_bytes[8] & Constants::UUID_VARIANT_MASK) | Constants::UUID_VARIANT_1;  // Variant 1

    // Convert to UUID string format
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < Constants::UUID_BYTE_LENGTH; i++) {
        if (i == Constants::UUID_POSITION_1 || i == Constants::UUID_POSITION_2 || i == Constants::UUID_POSITION_3 ||
            i == Constants::UUID_POSITION_4) {
            ss << "-";
        }
        ss << std::setw(2) << static_cast<int>(id_bytes[i]);
    }

    return ss.str();
}

}  // namespace Utils
}  // namespace AutoVibez
