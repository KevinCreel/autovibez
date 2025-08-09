#include "mix_downloader.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "console_output.hpp"
#include "constants.hpp"
#include "mix_metadata.hpp"
#include "mp3_analyzer.hpp"
#include "path_manager.hpp"
#include "path_utils.hpp"

using AutoVibez::Data::FileHandle;
using AutoVibez::Data::MixDownloader;

// Constants for better maintainability
constexpr size_t FILE_PROTOCOL_LENGTH = 7;
constexpr const char* INVALID_FILENAME_CHARS = "\\/:*?\"<>|";

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

// FileHandle RAII implementation
FileHandle::FileHandle(const std::string& path, const std::string& mode) : file_(nullptr) {
    file_ = fopen(path.c_str(), mode.c_str());
}

FileHandle::~FileHandle() {
    if (file_) {
        fclose(file_);
        file_ = nullptr;
    }
}

FileHandle::FileHandle(FileHandle&& other) noexcept : file_(other.file_) {
    other.file_ = nullptr;
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept {
    if (this != &other) {
        if (file_) {
            fclose(file_);
        }
        file_ = other.file_;
        other.file_ = nullptr;
    }
    return *this;
}

namespace AutoVibez {
namespace Data {

MixDownloader::MixDownloader(const std::string& mixes_dir) : mixes_dir(mixes_dir) {
    initializeCurl();
}

MixDownloader::~MixDownloader() {
    cleanupCurl();
}

bool MixDownloader::initializeCurl() {
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        // CURL might already be initialized, which is fine
        // We'll handle this gracefully
        setError(std::string(StringConstants::CURL_INIT_ERROR) + ": " + curl_easy_strerror(res));
        return false;
    }
    return true;
}

void MixDownloader::cleanupCurl() {
    // Cleanup CURL - this is safe to call even if not initialized
    // or if other parts of the application are using CURL
    curl_global_cleanup();
}

bool MixDownloader::isValidMixId(const std::string& mix_id) {
    if (mix_id.empty()) {
        return false;
    }

    // Check for invalid filename characters
    return mix_id.find_first_of(INVALID_FILENAME_CHARS) == std::string::npos;
}

bool MixDownloader::copyLocalFile(const std::string& source_path, const std::string& dest_path) {
    try {
        if (std::filesystem::copy_file(source_path, dest_path, std::filesystem::copy_options::overwrite_existing)) {
            return true;
        } else {
            setError(std::string(StringConstants::FILE_COPY_ERROR) + ": " + source_path);
            return false;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        setError(std::string(StringConstants::FILE_COPY_ERROR) + ": " + e.what());
        return false;
    }
}

bool MixDownloader::downloadFileWithCurl(const std::string& url, const std::string& file_path) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        setError(StringConstants::CURL_INIT_ERROR);
        return false;
    }

    // Use RAII for file handle
    FileHandle file_handle(file_path, "wb");
    if (!file_handle.isValid()) {
        setError(std::string(StringConstants::FILE_CREATE_ERROR) + ": " + file_path);
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file_handle.get());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, nullptr);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, Constants::DOWNLOAD_TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, Constants::MIN_DOWNLOAD_SPEED_BYTES_PER_SEC);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, Constants::DOWNLOAD_LOW_SPEED_TIME_SECONDS);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        setError(std::string(StringConstants::CURL_DOWNLOAD_ERROR) + ": " + curl_easy_strerror(res));
        // Clean up partial file
        std::filesystem::remove(file_path);
        return false;
    }

    return true;
}

bool MixDownloader::downloadMix(const Mix& mix) {
    clearError();

    if (mix.url.empty()) {
        setError(StringConstants::EMPTY_URL_ERROR);
        return false;
    }

    if (!isValidMixId(mix.id)) {
        setError(StringConstants::INVALID_MIX_ID_ERROR);
        return false;
    }

    std::string local_path = getLocalPathWithOriginalFilename(mix);

    if (isMixDownloaded(mix.id)) {
        return true;
    }

    std::filesystem::create_directories(mixes_dir);

    // Check if it's a local file using proper URL validation
    if (mix.url.substr(0, FILE_PROTOCOL_LENGTH) == StringConstants::FILE_PROTOCOL) {
        std::string source_path = mix.url.substr(FILE_PROTOCOL_LENGTH);
        return copyLocalFile(source_path, local_path);
    }

    // For HTTP/HTTPS URLs, validate them first
    if (!AutoVibez::Utils::UrlUtils::isValidUrl(mix.url)) {
        setError(std::string(StringConstants::INVALID_URL_ERROR) + ": " + mix.url);
        return false;
    }

    return downloadFileWithCurl(mix.url, local_path);
}

bool MixDownloader::isMixDownloaded(const std::string& mix_id) {
    if (!isValidMixId(mix_id)) {
        return false;
    }

    std::string local_path = getLocalPath(mix_id);
    if (std::filesystem::exists(local_path)) {
        return true;
    }

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
    if (!isValidMixId(mix_id)) {
        return "";
    }

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
                    return AutoVibez::Utils::PathUtils::joinPath(mixes_dir, filename);
                }
            }
        }
    }

    return AutoVibez::Utils::PathUtils::joinPath(mixes_dir, mix_id + StringConstants::MP3_EXTENSION);
}

std::string MixDownloader::getTemporaryPath(const std::string& mix_id) {
    if (!isValidMixId(mix_id)) {
        return "";
    }

    return AutoVibez::Utils::PathUtils::joinPath(mixes_dir, mix_id + ".tmp");
}

std::string MixDownloader::getLocalPathWithOriginalFilename(const Mix& mix) {
    if (mix.original_filename.empty()) {
        return getLocalPath(mix.id);
    }
    return AutoVibez::Utils::PathUtils::joinPath(mixes_dir, mix.original_filename);
}

std::string MixDownloader::getLastError() const {
    return last_error;
}

bool MixDownloader::downloadMixWithTitleNaming(const Mix& mix, AutoVibez::Audio::MP3Analyzer* mp3_analyzer) {
    clearError();

    if (mix.url.empty()) {
        setError(StringConstants::EMPTY_URL_ERROR);
        AutoVibez::Utils::ConsoleOutput::error("Download failed: Empty URL for " + mix.title);
        return false;
    }

    if (!isValidMixId(mix.id)) {
        setError(StringConstants::INVALID_MIX_ID_ERROR);
        AutoVibez::Utils::ConsoleOutput::error("Download failed: Invalid mix ID for " + mix.title);
        return false;
    }

    if (!mp3_analyzer) {
        setError(StringConstants::MP3_ANALYZER_REQUIRED_ERROR);
        AutoVibez::Utils::ConsoleOutput::error("Download failed: MP3 analyzer not available");
        return false;
    }

    std::string temp_path = getTemporaryPath(mix.id);
    std::string final_path = getLocalPath(mix.id);

    if (isMixDownloaded(mix.id)) {
        return true;
    }

    AutoVibez::Utils::ConsoleOutput::info("Downloading: " + mix.title);

    std::filesystem::create_directories(mixes_dir);

    if (mix.url.substr(0, FILE_PROTOCOL_LENGTH) == StringConstants::FILE_PROTOCOL) {
        std::string source_path = mix.url.substr(FILE_PROTOCOL_LENGTH);
        if (copyLocalFile(source_path, temp_path)) {
            AutoVibez::Audio::MP3Metadata mp3_metadata = mp3_analyzer->analyzeFile(temp_path);
            if (!mp3_metadata.title.empty()) {
                std::string safe_title = AutoVibez::Utils::PathUtils::createSafeFilename(mp3_metadata.title);
                final_path = AutoVibez::Utils::PathUtils::joinPath(mixes_dir, safe_title + ".mp3");
            }

            if (std::filesystem::exists(temp_path)) {
                std::filesystem::rename(temp_path, final_path);
            }
            AutoVibez::Utils::ConsoleOutput::success("Downloaded: " + mix.title);
            return true;
        } else {
            AutoVibez::Utils::ConsoleOutput::error("Failed to copy local file: " + mix.title);
            return false;
        }
    }

    if (!AutoVibez::Utils::UrlUtils::isValidUrl(mix.url)) {
        setError(std::string(StringConstants::INVALID_URL_ERROR) + ": " + mix.url);
        AutoVibez::Utils::ConsoleOutput::error("Download failed: Invalid URL for " + mix.title);
        return false;
    }

    if (!downloadFileWithCurl(mix.url, temp_path)) {
        AutoVibez::Utils::ConsoleOutput::error("Download failed: " + mix.title);
        return false;
    }

    AutoVibez::Audio::MP3Metadata mp3_metadata = mp3_analyzer->analyzeFile(temp_path);
    if (!mp3_metadata.title.empty()) {
        std::string safe_title = AutoVibez::Utils::PathUtils::createSafeFilename(mp3_metadata.title);
        final_path = AutoVibez::Utils::PathUtils::joinPath(mixes_dir, safe_title + StringConstants::MP3_EXTENSION);

        std::string mapping_file = PathManager::getFileMappingsPath();
        std::ofstream mapping(mapping_file, std::ios::app);
        if (mapping.is_open()) {
            mapping << mix.id << ":" << safe_title << StringConstants::MP3_EXTENSION << std::endl;
            mapping.close();
        }
    }

    if (std::filesystem::exists(temp_path)) {
        std::filesystem::rename(temp_path, final_path);
    }

    AutoVibez::Utils::ConsoleOutput::success("Downloaded: " + mix.title);
    return true;
}

}  // namespace Data
}  // namespace AutoVibez