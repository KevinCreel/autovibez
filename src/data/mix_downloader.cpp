#include "mix_downloader.hpp"

#include <curl/curl.h>

#include <filesystem>
#include <fstream>

#include "constants.hpp"
#include "mix_metadata.hpp"
#include "mp3_analyzer.hpp"
#include "path_manager.hpp"

using AutoVibez::Data::MixDownloader;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append(static_cast<const char*>(contents), size * nmemb);
    return size * nmemb;
}

static size_t WriteFileCallback(void* contents, size_t size, size_t nmemb, FILE* file) {
    return fwrite(contents, size, nmemb, file);
}

static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    (void)clientp, (void)ultotal, (void)ulnow, (void)dltotal, (void)dlnow;
    return 0;
}

namespace AutoVibez {
namespace Data {

MixDownloader::MixDownloader(const std::string& mixes_dir) : mixes_dir(mixes_dir) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

MixDownloader::~MixDownloader() {
    curl_global_cleanup();
}

bool MixDownloader::downloadMix(const Mix& mix) {
    last_error.clear();

    if (mix.url.empty()) {
        last_error = "Mix URL is empty";
        return false;
    }

    std::string local_path = getLocalPath(mix.id);

    // Check if already downloaded
    if (isMixDownloaded(mix.id)) {
        return true;
    }

    // Create cache directory if it doesn't exist
    std::filesystem::create_directories(mixes_dir);

    // Handle local file URLs
    if (mix.url.substr(0, 7) == StringConstants::FILE_PROTOCOL) {
        std::string source_path = mix.url.substr(7);
        // Copy local file
        if (std::filesystem::copy_file(source_path, local_path, std::filesystem::copy_options::overwrite_existing)) {
            return true;
        }
    }

    // Handle HTTP downloads
    CURL* curl = curl_easy_init();
    if (!curl) {
        last_error = "Failed to initialize CURL";
        return false;
    }

    FILE* file = fopen(local_path.c_str(), "wb");
    if (!file) {
        last_error = "Failed to create local file: " + local_path;
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, mix.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, nullptr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, Constants::DOWNLOAD_TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, Constants::MIN_DOWNLOAD_SPEED_BYTES_PER_SEC);  // 1KB/s minimum
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, Constants::DOWNLOAD_LOW_SPEED_TIME_SECONDS);

    CURLcode res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        last_error = "Download failed: " + std::string(curl_easy_strerror(res));
        std::filesystem::remove(local_path);  // Clean up failed download
        return false;
    }

    return true;
}

bool MixDownloader::isMixDownloaded(const std::string& mix_id) {
    // Check if the file exists with the hash-based name
    std::string local_path = getLocalPath(mix_id);
    if (std::filesystem::exists(local_path)) {
        return true;
    }

    // Check if there's a mapping file that tracks renamed files
    std::string mapping_file = PathManager::getFileMappingsPath();
    if (std::filesystem::exists(mapping_file)) {
        std::ifstream file(mapping_file);
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string stored_id = line.substr(0, pos);
                std::string filename = line.substr(pos + 1);
                if (stored_id == mix_id) {
                    std::string mapped_path = mixes_dir + "/" + filename;
                    if (std::filesystem::exists(mapped_path)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

std::string MixDownloader::getLocalPath(const std::string& mix_id) {
    // First check if there's a mapping for this mix ID
    std::string mapping_file = PathManager::getFileMappingsPath();
    if (std::filesystem::exists(mapping_file)) {
        std::ifstream file(mapping_file);
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string stored_id = line.substr(0, pos);
                std::string filename = line.substr(pos + 1);
                if (stored_id == mix_id) {
                    return mixes_dir + "/" + filename;
                }
            }
        }
    }

    // Fall back to hash-based naming
    return mixes_dir + "/" + mix_id + StringConstants::MP3_EXTENSION;
}

std::string MixDownloader::getTemporaryPath(const std::string& mix_id) {
    return mixes_dir + "/" + mix_id + ".tmp";
}

std::string MixDownloader::getLocalPathWithOriginalFilename(const Mix& mix) {
    if (mix.original_filename.empty()) {
        return getLocalPath(mix.id);
    }
    return mixes_dir + "/" + mix.original_filename;
}

bool MixDownloader::downloadMixWithTitleNaming(const Mix& mix, AutoVibez::Audio::MP3Analyzer* mp3_analyzer) {
    last_error.clear();

    if (mix.url.empty()) {
        last_error = "Mix URL is empty";
        return false;
    }

    if (!mp3_analyzer) {
        last_error = "MP3Analyzer is required for title-based naming";
        return false;
    }

    std::string temp_path = getTemporaryPath(mix.id);
    std::string final_path = getLocalPath(mix.id);  // Will be renamed later

    // Check if already downloaded
    if (isMixDownloaded(mix.id)) {
        return true;
    }

    // Create cache directory if it doesn't exist
    std::filesystem::create_directories(mixes_dir);

    // Handle local file URLs
    if (mix.url.substr(0, 7) == StringConstants::FILE_PROTOCOL) {
        std::string source_path = mix.url.substr(7);
        // Copy local file to temp location
        if (std::filesystem::copy_file(source_path, temp_path, std::filesystem::copy_options::overwrite_existing)) {
            // Analyze the file to get the title
            AutoVibez::Audio::MP3Metadata mp3_metadata = mp3_analyzer->analyzeFile(temp_path);
            if (!mp3_metadata.title.empty()) {
                // Create a safe filename from the title
                std::string safe_title = createSafeFilename(mp3_metadata.title);
                final_path = mixes_dir + "/" + safe_title + ".mp3";
            }

            // Rename temp file to final location
            if (std::filesystem::exists(temp_path)) {
                std::filesystem::rename(temp_path, final_path);
            }
            return true;
        }
    }

    // Handle HTTP downloads
    CURL* curl = curl_easy_init();
    if (!curl) {
        last_error = "Failed to initialize CURL";
        return false;
    }

    FILE* file = fopen(temp_path.c_str(), "wb");
    if (!file) {
        last_error = "Failed to create temporary file: " + temp_path;
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, mix.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, nullptr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, Constants::DOWNLOAD_TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, Constants::MIN_DOWNLOAD_SPEED_BYTES_PER_SEC);  // 1KB/s minimum
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, Constants::DOWNLOAD_LOW_SPEED_TIME_SECONDS);

    CURLcode res = curl_easy_perform(curl);

    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        last_error = "Download failed: " + std::string(curl_easy_strerror(res));
        std::filesystem::remove(temp_path);  // Clean up failed download
        return false;
    }

    // Analyze the downloaded file to get the title
    AutoVibez::Audio::MP3Metadata mp3_metadata = mp3_analyzer->analyzeFile(temp_path);
    if (!mp3_metadata.title.empty()) {
        // Create a safe filename from the title
        std::string safe_title = createSafeFilename(mp3_metadata.title);
        final_path = mixes_dir + "/" + safe_title + StringConstants::MP3_EXTENSION;

        // Save the mapping for future reference
        std::string mapping_file = PathManager::getFileMappingsPath();
        std::ofstream mapping(mapping_file, std::ios::app);
        if (mapping.is_open()) {
            mapping << mix.id << ":" << safe_title << StringConstants::MP3_EXTENSION << std::endl;
            mapping.close();
        }
    }

    // Rename temp file to final location
    if (std::filesystem::exists(temp_path)) {
        std::filesystem::rename(temp_path, final_path);
    }

    return true;
}

std::string MixDownloader::createSafeFilename(const std::string& title) {
    std::string safe_filename = title;

    // Replace only truly invalid filename characters (spaces are valid on most filesystems)
    const std::string invalid_chars = "<>:\"/\\|?*";
    for (char c : invalid_chars) {
        size_t pos = 0;
        while ((pos = safe_filename.find(c, pos)) != std::string::npos) {
            safe_filename.replace(pos, 1, "_");
            pos++;
        }
    }

    // Remove trailing underscores
    while (!safe_filename.empty() && safe_filename.back() == '_') {
        safe_filename.pop_back();
    }

    // Limit length to avoid filesystem issues
    if (safe_filename.length() > Constants::MAX_FILENAME_LENGTH) {
        safe_filename = safe_filename.substr(0, Constants::MAX_FILENAME_LENGTH);
    }

    return safe_filename;
}

}  // namespace Data
}  // namespace AutoVibez