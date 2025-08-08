#pragma once

#include <string>

#include "database_interfaces.hpp"
#include "mix_metadata.hpp"

namespace AutoVibez {
namespace Data {

/**
 * @brief Validates Mix objects for database operations
 *
 * This class encapsulates all validation logic for Mix objects,
 * ensuring data integrity before database operations.
 */
class MixValidator {
public:
    /**
     * @brief Validate a Mix object for database insertion/update
     * @param mix Mix object to validate
     * @return ValidationResult indicating success or failure with error message
     */
    ValidationResult validate(const Mix& mix) const;

    /**
     * @brief Validate Mix ID format
     * @param id Mix ID to validate
     * @return ValidationResult indicating success or failure
     */
    ValidationResult validateId(const std::string& id) const;

    /**
     * @brief Validate Mix title
     * @param title Mix title to validate
     * @param id Mix ID for context in error messages
     * @return ValidationResult indicating success or failure
     */
    ValidationResult validateTitle(const std::string& title, const std::string& id) const;

    /**
     * @brief Validate Mix artist
     * @param artist Mix artist to validate
     * @return ValidationResult indicating success or failure
     */
    ValidationResult validateArtist(const std::string& artist) const;

    /**
     * @brief Validate Mix genre
     * @param genre Mix genre to validate
     * @return ValidationResult indicating success or failure
     */
    ValidationResult validateGenre(const std::string& genre) const;

    /**
     * @brief Validate Mix duration
     * @param duration_seconds Duration in seconds to validate
     * @return ValidationResult indicating success or failure
     */
    ValidationResult validateDuration(int duration_seconds) const;

private:
    /**
     * @brief Check if string contains only valid characters
     * @param str String to check
     * @return True if valid, false otherwise
     */
    bool hasValidCharacters(const std::string& str) const;

    /**
     * @brief Check if string is not just whitespace
     * @param str String to check
     * @return True if not just whitespace, false otherwise
     */
    bool isNotJustWhitespace(const std::string& str) const;
};

}  // namespace Data
}  // namespace AutoVibez
