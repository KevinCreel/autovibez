#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "uuid_utils.hpp"
#include "error_handler.hpp"
// Mix struct is defined in this file

namespace AutoVibez {
namespace Data {

/**
 * @brief Represents a DJ mix with all its metadata
 */
struct Mix {
    std::string id;                    // Unique identifier
    std::string title;                 // Mix title
    std::string artist;                // Artist/DJ name
    std::string genre;                 // Music genre
    std::string url;                   // Download URL
    std::string original_filename;     // Original filename from URL
    std::string local_path;            // Local file path (if downloaded)
    int duration_seconds;              // Duration in seconds

    std::vector<std::string> tags;     // Tags/keywords
    std::string description;           // Description
    std::string date_added;            // When added to database
    std::string last_played;           // Last played timestamp
    int play_count;                    // Number of times played
    bool is_favorite;                  // User favorite flag
    
    Mix() : duration_seconds(0), play_count(0), is_favorite(false) {}
};

/**
 * @brief Manages loading and parsing of mix metadata from YAML
 */
class MixMetadata : public AutoVibez::Utils::ErrorHandler {
public:
    MixMetadata();
    ~MixMetadata();
    
    /**
     * @brief Load mix metadata from a YAML file
     * @param yaml_url URL or file path to YAML file
     * @return Vector of loaded mixes
     */
    std::vector<Mix> loadFromYaml(const std::string& yaml_url);
    
    /**
     * @brief Load mix metadata from a local YAML file
     * @param file_path Path to local YAML file
     * @return Vector of loaded mixes
     */
    std::vector<Mix> loadFromLocalFile(const std::string& file_path);
    
    /**
     * @brief Load mix metadata from a remote YAML file
     * @param url URL to remote YAML file
     * @return Vector of loaded mixes
     */
    std::vector<Mix> loadFromRemoteFile(const std::string& url);
    


private:
    
    /**
     * @brief Parse a single mix from YAML node
     * @param mix_node YAML node containing mix data
     * @return Parsed Mix object
     */
    Mix parseMixFromYaml(const YAML::Node& mix_node);
    
    /**
     * @brief Validate a mix object
     * @param mix Mix to validate
     * @return True if valid, false otherwise
     */
    bool validateMix(const Mix& mix);

    /**
     * @brief Extract filename from URL
     * @param url URL to extract filename from
     * @return Extracted filename or empty string if not found
     */
    std::string extractFilenameFromUrl(const std::string& url);
};

} // namespace Data
} // namespace AutoVibez 