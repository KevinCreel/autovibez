#include "mix_validator.hpp"

#include <algorithm>
#include <cctype>

namespace AutoVibez {
namespace Data {

ValidationResult MixValidator::validate(const Mix& mix) const {
    // Validate required fields
    auto id_result = validateId(mix.id);
    if (!id_result) {
        return id_result;
    }

    auto title_result = validateTitle(mix.title, mix.id);
    if (!title_result) {
        return title_result;
    }

    auto artist_result = validateArtist(mix.artist);
    if (!artist_result) {
        return artist_result;
    }

    auto genre_result = validateGenre(mix.genre);
    if (!genre_result) {
        return genre_result;
    }

    auto duration_result = validateDuration(mix.duration_seconds);
    if (!duration_result) {
        return duration_result;
    }

    return ValidationResult(true);
}

ValidationResult MixValidator::validateId(const std::string& id) const {
    if (id.empty()) {
        return ValidationResult(false, "Mix ID cannot be empty");
    }

    if (!hasValidCharacters(id)) {
        return ValidationResult(false, "Mix ID contains invalid characters");
    }

    if (!isNotJustWhitespace(id)) {
        return ValidationResult(false, "Mix ID cannot be just whitespace");
    }

    // Check for reasonable length (UUIDs are typically 36 characters)
    if (id.length() > 100) {
        return ValidationResult(false, "Mix ID is too long (max 100 characters)");
    }

    return ValidationResult(true);
}

ValidationResult MixValidator::validateTitle(const std::string& title, const std::string& id) const {
    if (title.empty()) {
        return ValidationResult(false, "Mix title cannot be empty");
    }

    if (!isNotJustWhitespace(title)) {
        return ValidationResult(false, "Mix title cannot be just whitespace");
    }

    // Title cannot be the same as ID (business rule from original code)
    if (title == id) {
        return ValidationResult(false, "Mix title cannot be the same as ID");
    }

    // Check for reasonable length
    if (title.length() > 500) {
        return ValidationResult(false, "Mix title is too long (max 500 characters)");
    }

    return ValidationResult(true);
}

ValidationResult MixValidator::validateArtist(const std::string& artist) const {
    if (artist.empty()) {
        return ValidationResult(false, "Mix artist cannot be empty");
    }

    if (!isNotJustWhitespace(artist)) {
        return ValidationResult(false, "Mix artist cannot be just whitespace");
    }

    // Business rule from original code: artist cannot be "Unknown Artist"
    if (artist == "Unknown Artist") {
        return ValidationResult(false, "Mix artist cannot be 'Unknown Artist'");
    }

    // Check for reasonable length
    if (artist.length() > 200) {
        return ValidationResult(false, "Mix artist is too long (max 200 characters)");
    }

    return ValidationResult(true);
}

ValidationResult MixValidator::validateGenre(const std::string& genre) const {
    if (genre.empty()) {
        return ValidationResult(false, "Mix genre cannot be empty");
    }

    if (!isNotJustWhitespace(genre)) {
        return ValidationResult(false, "Mix genre cannot be just whitespace");
    }

    // Check for reasonable length
    if (genre.length() > 100) {
        return ValidationResult(false, "Mix genre is too long (max 100 characters)");
    }

    return ValidationResult(true);
}

ValidationResult MixValidator::validateDuration(int duration_seconds) const {
    if (duration_seconds <= 0) {
        return ValidationResult(false, "Mix duration must be greater than 0 seconds");
    }

    // Reasonable upper bound (24 hours = 86400 seconds)
    if (duration_seconds > 86400) {
        return ValidationResult(false, "Mix duration is too long (max 24 hours)");
    }

    return ValidationResult(true);
}

bool MixValidator::hasValidCharacters(const std::string& str) const {
    // For now, allow most printable characters
    // This could be made more restrictive based on requirements
    return std::all_of(str.begin(), str.end(), [](unsigned char c) { return std::isprint(c) || std::isspace(c); });
}

bool MixValidator::isNotJustWhitespace(const std::string& str) const {
    return std::any_of(str.begin(), str.end(), [](unsigned char c) { return !std::isspace(c); });
}

}  // namespace Data
}  // namespace AutoVibez
