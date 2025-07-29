#pragma once

#include <string>
#include <vector>
#include <memory>
#include <gmock/gmock.h>
#include "mix_metadata.hpp"

/**
 * @brief Mock mix downloader class for testing
 */
class MockMixDownloader {
public:
    MOCK_METHOD(bool, downloadMix, (const Mix& mix, const std::string& download_path), ());
    MOCK_METHOD(bool, downloadMixById, (const std::string& mix_id, const std::string& download_path), ());
    MOCK_METHOD(bool, downloadMixByUrl, (const std::string& url, const std::string& download_path), ());
    MOCK_METHOD(bool, isDownloading, (), (const));
    MOCK_METHOD(double, getDownloadProgress, (), (const));
    MOCK_METHOD(std::string, getDownloadStatus, (), (const));
    MOCK_METHOD(void, cancelDownload, (), ());
    MOCK_METHOD(bool, isMixDownloaded, (const std::string& mix_id), (const));
    MOCK_METHOD(std::string, getLocalPath, (const std::string& mix_id), (const));
    MOCK_METHOD(bool, deleteLocalFile, (const std::string& mix_id), ());
    MOCK_METHOD(std::vector<std::string>, getDownloadedMixIds, (), (const));
    MOCK_METHOD(size_t, getDownloadedSize, (const std::string& mix_id), (const));
    MOCK_METHOD(size_t, getTotalSize, (const std::string& mix_id), (const));
    MOCK_METHOD(bool, verifyDownload, (const std::string& mix_id), (const));
    MOCK_METHOD(std::string, getLastError, (), (const));
    MOCK_METHOD(bool, isSuccess, (), (const));
    
    // Helper methods for testing
    void setMockDownloadProgress(double progress) {
        mock_download_progress = progress;
    }
    
    void setMockDownloadStatus(const std::string& status) {
        mock_download_status = status;
    }
    
    void setMockIsDownloading(bool downloading) {
        mock_is_downloading = downloading;
    }
    
    void setMockDownloadedMixIds(const std::vector<std::string>& ids) {
        mock_downloaded_ids = ids;
    }
    
    void setMockLocalPaths(const std::map<std::string, std::string>& paths) {
        mock_local_paths = paths;
    }

private:
    double mock_download_progress = 0.0;
    std::string mock_download_status = "Idle";
    bool mock_is_downloading = false;
    std::vector<std::string> mock_downloaded_ids;
    std::map<std::string, std::string> mock_local_paths;
}; 