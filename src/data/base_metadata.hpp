#pragma once

#include <string>

#include "constants.hpp"
#include <vector>

namespace AutoVibez {
namespace Data {

/**
 * @brief Base class for metadata structures with common fields
 *
 * This class contains the shared fields between MP3Metadata and Mix structures,
 * reducing code duplication and providing a consistent interface for metadata handling.
 */
struct BaseMetadata {
    std::string id;                 ///< Unique identifier
    std::string title;              ///< Track title
    std::string artist;             ///< Artist name
    std::string genre;              ///< Music genre
    std::vector<std::string> tags;  ///< User-defined tags
    std::string description;        ///< Track description
    std::string local_path;         ///< Local file path
    int duration_seconds;           ///< Track duration in seconds
    std::string date_added;         ///< Date when added to library
    std::string last_played;        ///< Date when last played
    int play_count;                 ///< Number of times played
    bool is_favorite;               ///< Whether track is marked as favorite

    /**
     * @brief Default constructor initializes all fields
     */
    BaseMetadata() : duration_seconds(0), play_count(0), is_favorite(false) {}

    /**
     * @brief Virtual destructor for proper inheritance
     */
    virtual ~BaseMetadata() = default;

    /**
     * @brief Check if the metadata has a valid ID
     * @return True if ID is not empty, false otherwise
     */
    bool hasValidId() const {
        return !id.empty();
    }

    /**
     * @brief Check if the metadata has a valid title
     * @return True if title is not empty, false otherwise
     */
    bool hasValidTitle() const {
        return !title.empty();
    }

    /**
     * @brief Check if the metadata has a valid local path
     * @return True if local_path is not empty, false otherwise
     */
    bool hasValidLocalPath() const {
        return !local_path.empty();
    }

    /**
     * @brief Check if the metadata has been played at least once
     * @return True if play_count > 0, false otherwise
     */
    bool hasBeenPlayed() const {
        return play_count > 0;
    }

    /**
     * @brief Check if the metadata has a valid duration
     * @return True if duration_seconds > 0, false otherwise
     */
    bool hasValidDuration() const {
        return duration_seconds > 0;
    }

    /**
     * @brief Get a display name for the track
     * @return Title if available, otherwise "Unknown Track"
     */
    std::string getDisplayName() const {
        return hasValidTitle() ? title : StringConstants::UNKNOWN_TRACK;
    }

    /**
     * @brief Get a display artist for the track
     * @return Artist if available, otherwise "Unknown Artist"
     */
    std::string getDisplayArtist() const {
        return !artist.empty() ? artist : StringConstants::UNKNOWN_ARTIST;
    }

    /**
     * @brief Check if the metadata is complete (has essential fields)
     * @return True if has valid ID, title, and local path, false otherwise
     */
    bool isComplete() const {
        return hasValidId() && hasValidTitle() && hasValidLocalPath();
    }
};

}  // namespace Data
}  // namespace AutoVibez
