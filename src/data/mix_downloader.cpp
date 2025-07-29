#include "mix_downloader.hpp"
#include "console_output.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <filesystem>

using AutoVibez::Data::MixDownloader;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append(static_cast<const char*>(contents), size * nmemb);
    return size * nmemb;
}

static size_t WriteFileCallback(void* contents, size_t size, size_t nmemb, FILE* file) {
    return fwrite(contents, size, nmemb, file);
}

static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    (void)clientp; (void)ultotal; (void)ulnow; // Parameters not used in current implementation
    if (dltotal > 0) {
        int progress = static_cast<int>((dlnow * 100) / dltotal);
        ConsoleOutput::output("\r📥 Downloading: %d%% (%ld/%ld bytes)", progress, dlnow, dltotal);
        fflush(stdout);
    }
    return 0;
}

namespace AutoVibez {
namespace Data {

MixDownloader::MixDownloader(const std::string& cacheDir) : cache_dir(cacheDir) {
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
    std::filesystem::create_directories(cache_dir);
    
    // Handle local file URLs
    if (mix.url.substr(0, 7) == "file://") {
        std::string source_path = mix.url.substr(7);
        ConsoleOutput::output("📁 Copying local file: %s", source_path.c_str());
        
        try {
            std::filesystem::copy_file(source_path, local_path, std::filesystem::copy_options::overwrite_existing);
            ConsoleOutput::output("✅ Local file copied: %s", local_path.c_str());
            return true;
        } catch (const std::exception& e) {
            last_error = "Failed to copy local file: " + std::string(e.what());
            return false;
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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // 5 minutes timeout
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1000L); // 1KB/s minimum
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 60L); // 60 seconds
    
    CURLcode res = curl_easy_perform(curl);
    
    fclose(file);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        last_error = "Download failed: " + std::string(curl_easy_strerror(res));
        std::filesystem::remove(local_path); // Clean up failed download
        return false;
    }
    
    return true;
}

bool MixDownloader::isMixDownloaded(const std::string& mix_id) {
    std::string local_path = getLocalPath(mix_id);
    return std::filesystem::exists(local_path);
}

std::string MixDownloader::getLocalPath(const std::string& mix_id) {
    return cache_dir + "/" + mix_id + ".mp3";
}

} // namespace Data
} // namespace AutoVibez 