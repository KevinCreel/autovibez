#pragma once

#include <yaml-cpp/yaml.h>

#include <string>
#include <vector>

#include "base_metadata.hpp"
#include "error_handler.hpp"
#include "url_utils.hpp"
#include "uuid_utils.hpp"
// Mix struct is defined in this file

namespace AutoVibez {
namespace Data {

/**
 * @brief Represents a DJ mix with all its metadata
 */
struct Mix : public BaseMetadata {
    std::string url;                // Download URL
    std::string original_filename;  // Original filename from URL

    Mix() {}
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

public:
    /**
     * @brief Validate a mix object
     * @param mix Mix to validate
     * @return True if valid, false otherwise
     */
    bool validateMix(const Mix& mix);

private:
};

}  // namespace Data
}  // namespace AutoVibez