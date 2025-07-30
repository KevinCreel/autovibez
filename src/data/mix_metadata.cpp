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
#include "string_utils.hpp"
#include "resource_guard.hpp"
#include "constants.hpp"
#include "config_defaults.hpp"

using AutoVibez::Data::Mix;
using AutoVibez::Data::MixMetadata;

namespace AutoVibez {
namespace Data {

// Callback function for CURL to write data to string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append(static_cast<const char*>(contents), size * nmemb);
    return size * nmemb;
}

MixMetadata::MixMetadata() : success(true) {
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

MixMetadata::~MixMetadata() {
    // Cleanup CURL
    curl_global_cleanup();
}

std::vector<Mix> MixMetadata::loadFromYaml(const std::string& yaml_url) {
    success = true;
    last_error.clear();
    
    // Check if it's a URL or local file
    if (yaml_url.substr(0, 7) == "http://" || yaml_url.substr(0, 8) == "https://") {
        return loadFromRemoteFile(yaml_url);
    } else {
        return loadFromLocalFile(yaml_url);
    }
}

std::vector<Mix> MixMetadata::loadFromLocalFile(const std::string& file_path) {
    success = true;
    last_error.clear();
    std::vector<Mix> mixes;
    
    try {
        // Check if file exists and is readable
        std::ifstream file(file_path);
        if (!file.is_open()) {
            last_error = "Cannot open file: " + file_path;
            success = false;
            return mixes;
        }
        
        // Read file content to check for basic YAML structure
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        if (content.empty()) {
            last_error = "File is empty: " + file_path;
            success = false;
            return mixes;
        }
        
        // Check for basic YAML structure
        if (content.find("mixes:") == std::string::npos) {
            last_error = "No 'mixes:' section found in file";
            success = false;
            return mixes;
        }
        
        YAML::Node config = YAML::Load(content);
        
        if (!config["mixes"]) {
            last_error = "No 'mixes' section found in YAML file";
            success = false;
            return mixes;
        }
        
        YAML::Node mixes_node = config["mixes"];
        if (!mixes_node.IsSequence()) {
            last_error = "Invalid 'mixes' section - expected sequence";
            success = false;
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
        last_error = "YAML parsing error: " + std::string(e.what());
        return std::vector<Mix>();
    } catch (const std::exception& e) {
        last_error = "File reading error: " + std::string(e.what());
        success = false;
    }
    
    return mixes;
}

std::vector<Mix> MixMetadata::loadFromRemoteFile(const std::string& url) {
    success = true;
    last_error.clear();
    std::vector<Mix> mixes;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        last_error = "Failed to initialize CURL";
        success = false;
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
        last_error = "HTTP request failed: " + std::string(curl_easy_strerror(res));
        success = false;
        return mixes;
    }
    
    // Check if response is empty or too small
    if (response.empty() || response.length() < 10) {
        last_error = "Empty or invalid response from server";
        success = false;
        return mixes;
    }
    
    try {
        YAML::Node config = YAML::Load(response);
        
        if (!config["mixes"]) {
            last_error = "No 'mixes' section found in YAML response";
            success = false;
            return mixes;
        }
        
        YAML::Node mixes_node = config["mixes"];
        if (!mixes_node.IsSequence()) {
            last_error = "Invalid 'mixes' section - expected sequence";
            success = false;
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
        last_error = "YAML parsing error: " + std::string(e.what());
        success = false;
    } catch (const std::exception& e) {
        last_error = "Response parsing error: " + std::string(e.what());
        success = false;
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
        mix.id = generateIdFromUrl(mix.url);
    } else {
        // Object format (for backward compatibility)
        if (mix_node["id"] && mix_node["id"].IsScalar()) {
            mix.id = mix_node["id"].as<std::string>();
        }
        if (mix_node["url"] && mix_node["url"].IsScalar()) {
            mix.url = mix_node["url"].as<std::string>();
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

std::string MixMetadata::generateIdFromUrl(const std::string& url) {
    // Generate a deterministic UUID v5 based on URL hash (consistent with MP3Analyzer)
    std::hash<std::string> hasher;
    size_t hash = hasher(url);
    
    // Use the hash to generate a deterministic UUID v5 (name-based)
    unsigned char uuid_bytes[16];
    
    // Use first 16 bytes of hash (or repeat if shorter)
    for (int i = 0; i < 16; i++) {
        uuid_bytes[i] = (hash >> (i % 8 * 8)) & 0xFF;
    }
    
    // Set version (5) and variant bits for deterministic UUID
    uuid_bytes[6] = (uuid_bytes[6] & 0x0F) | 0x50;  // Version 5
    uuid_bytes[8] = (uuid_bytes[8] & 0x3F) | 0x80;  // Variant 1
    
    // Convert to UUID string format
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (int i = 0; i < 16; i++) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            ss << "-";
        }
        ss << std::setw(2) << static_cast<int>(uuid_bytes[i]);
    }
    
    return ss.str();
}

bool MixMetadata::validateMix(const Mix& mix) {
    // For minimal YAML format, only URL is required
    if (mix.url.empty()) {
        last_error = "Mix missing required field: url";
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