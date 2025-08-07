#include "mix_metadata.hpp"
#include <yaml-cpp/yaml.h>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <functional>
#include <random>
#include <iomanip>
#include "path_manager.hpp"
#include "constants.hpp"

using AutoVibez::Data::Mix;
using AutoVibez::Data::MixMetadata;

namespace AutoVibez {
namespace Data {

// Callback function for CURL to write data to string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append(static_cast<const char*>(contents), size * nmemb);
    return size * nmemb;
}

MixMetadata::MixMetadata() {
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

MixMetadata::~MixMetadata() {
    // Cleanup CURL
    curl_global_cleanup();
}

std::vector<Mix> MixMetadata::loadFromYaml(const std::string& yaml_url) {
    clearError();
    
    // Check if it's a URL or local file
    if (yaml_url.substr(0, 7) == "http://" || yaml_url.substr(0, 8) == "https://") {
        return loadFromRemoteFile(yaml_url);
    } else {
        return loadFromLocalFile(yaml_url);
    }
}

std::vector<Mix> MixMetadata::loadFromLocalFile(const std::string& file_path) {
    clearError();
    std::vector<Mix> mixes;
    
    try {
        // Check if file exists and is readable
        std::ifstream file(file_path);
        if (!file.is_open()) {
            setError("Cannot open file: " + file_path);
            return mixes;
        }
        
        // Read file content to check for basic YAML structure
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        if (content.empty()) {
            setError("File is empty: " + file_path);
            return mixes;
        }
        
        // Check for basic YAML structure
        if (content.find("mixes:") == std::string::npos) {
            setError("No 'mixes:' section found in file");
            return mixes;
        }
        
        YAML::Node config = YAML::Load(content);
        
        if (!config["mixes"]) {
            setError("No 'mixes' section found in YAML file");
            return mixes;
        }
        
        YAML::Node mixes_node = config["mixes"];
        if (!mixes_node.IsSequence()) {
            setError("Invalid 'mixes' section - expected sequence");
            return mixes;
        }
        
        for (const auto& mix_node : mixes_node) {
            try {
                Mix mix = parseMixFromYaml(mix_node);
                if (validateMix(mix)) {
                    mixes.push_back(mix);
                }
            } catch (const std::exception& e) {
                // Skip invalid mixes but continue processing
                continue;
            }
        }
        
        return mixes;
    } catch (const YAML::Exception& e) {
        setError("YAML parsing error: " + std::string(e.what()));
        return std::vector<Mix>();
    } catch (const std::exception& e) {
        setError("File reading error: " + std::string(e.what()));
    }
    
    return mixes;
}

std::vector<Mix> MixMetadata::loadFromRemoteFile(const std::string& url) {
    clearError();
    std::vector<Mix> mixes;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        setError("Failed to initialize CURL");
        return mixes;
    }
    
    std::string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "AutoVibez/1.0");
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        setError("HTTP request failed: " + std::string(curl_easy_strerror(res)));
        return mixes;
    }
    
    // Check if response is empty or too small
    if (response.empty() || response.length() < 10) {
        setError("Empty or invalid response from server");
        return mixes;
    }
    
    try {
        YAML::Node config = YAML::Load(response);
        
        if (!config["mixes"]) {
            setError("No 'mixes' section found in YAML response");
            return mixes;
        }
        
        YAML::Node mixes_node = config["mixes"];
        if (!mixes_node.IsSequence()) {
            setError("Invalid 'mixes' section - expected sequence");
            return mixes;
        }
        
        for (const auto& mix_node : mixes_node) {
            try {
                Mix mix = parseMixFromYaml(mix_node);
                if (validateMix(mix)) {
                    mixes.push_back(mix);
                }
            } catch (const std::exception& e) {
                // Skip invalid mixes but continue processing
                continue;
            }
        }
        
        return mixes;
    } catch (const YAML::Exception& e) {
        setError("YAML parsing error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        setError("Response parsing error: " + std::string(e.what()));
    }
    
    return mixes;
}

Mix MixMetadata::parseMixFromYaml(const YAML::Node& mix_node) {
    Mix mix;
    
    // Handle both string URLs and object format for backward compatibility
    if (mix_node.IsScalar()) {
        // Simple URL string format
        mix.url = mix_node.as<std::string>();
        // Generate ID from URL
        mix.id = AutoVibez::Utils::UuidUtils::generateIdFromUrl(mix.url);
        // Extract original filename from URL
        mix.original_filename = AutoVibez::Utils::UrlUtils::extractFilenameFromUrl(mix.url);
    } else {
        // Object format (for backward compatibility)
        if (mix_node["id"] && mix_node["id"].IsScalar()) {
            mix.id = mix_node["id"].as<std::string>();
        }
        if (mix_node["url"] && mix_node["url"].IsScalar()) {
            mix.url = mix_node["url"].as<std::string>();
            // Extract original filename from URL
            mix.original_filename = AutoVibez::Utils::UrlUtils::extractFilenameFromUrl(mix.url);
        }
        
        // Optional fields (will be filled in from MP3 analysis)
        if (mix_node["title"] && mix_node["title"].IsScalar()) {
            mix.title = mix_node["title"].as<std::string>();
        }
        if (mix_node["artist"] && mix_node["artist"].IsScalar()) {
            mix.artist = mix_node["artist"].as<std::string>();
        }
        if (mix_node["genre"] && mix_node["genre"].IsScalar()) {
            mix.genre = mix_node["genre"].as<std::string>();
        }
        if (mix_node["duration_seconds"] && mix_node["duration_seconds"].IsScalar()) {
            mix.duration_seconds = mix_node["duration_seconds"].as<int>();
        }
        if (mix_node["description"] && mix_node["description"].IsScalar()) {
            mix.description = mix_node["description"].as<std::string>();
        }
        
        // Optional fields
        if (mix_node["tags"] && mix_node["tags"].IsSequence()) {
            for (const auto& tag : mix_node["tags"]) {
                if (tag.IsScalar()) {
                    mix.tags.push_back(tag.as<std::string>());
                }
            }
        }
    }
    
    // Initialize user-specific fields

    mix.play_count = 0;
    mix.is_favorite = false;
    
    return mix;
}





bool MixMetadata::validateMix(const Mix& mix) {
    // For minimal YAML format, only URL is required
    if (mix.url.empty()) {
        setError("Mix missing required field: url");
        return false;
    }
    
    // ID is optional but recommended
    if (mix.id.empty()) {
        // Generate ID from URL if not provided
        // This will be handled by the MP3Analyzer later
    }
    
    // All other fields are optional and will be extracted from MP3 file
    return true;
}

} // namespace Data
} // namespace AutoVibez 