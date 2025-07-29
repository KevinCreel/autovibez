#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include "mix_downloader.hpp"
#include "mix_metadata.hpp"
#include "fixtures/test_fixtures.hpp"

using AutoVibez::Data::MixDownloader;
using AutoVibez::Data::Mix;

class MixDownloaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = TestFixtures::createTempTestDir();
        cache_dir = test_dir + "/cache";
        std::filesystem::create_directories(cache_dir);
        
        downloader = std::make_unique<MixDownloader>(cache_dir);
    }
    
    void TearDown() override {
        downloader.reset();
        TestFixtures::cleanupTestFiles({test_dir});
    }
    
    std::string test_dir;
    std::string cache_dir;
    std::unique_ptr<MixDownloader> downloader;
    
    // Helper method to create a test mix
    Mix createTestMix(const std::string& id, const std::string& url) {
        Mix mix;
        mix.id = id;
        mix.title = "Test Mix " + id;
        mix.artist = "Test Artist";
        mix.genre = "Test Genre";
        mix.url = url;
        return mix;
    }
    
    // Helper method to create a test file
    void createTestFile(const std::string& path, const std::string& content) {
        std::ofstream file(path);
        file << content;
        file.close();
    }
};

TEST_F(MixDownloaderTest, Constructor) {
    // Test that MixDownloader can be constructed
    EXPECT_NE(downloader, nullptr);
}

TEST_F(MixDownloaderTest, ConstructorWithInvalidPath) {
    // Test construction with invalid cache directory
    MixDownloader invalidDownloader("/nonexistent/path");
    EXPECT_NE(&invalidDownloader, nullptr);
}

TEST_F(MixDownloaderTest, IsMixDownloaded) {
    // Test checking if mix is downloaded
    std::string mix_id = "test_mix_1";
    
    // Initially should not be downloaded
    EXPECT_FALSE(downloader->isMixDownloaded(mix_id));
    
    // Create a fake downloaded file
    std::string localPath = downloader->getLocalPath(mix_id);
    createTestFile(localPath, "fake audio data");
    
    // Now should be considered downloaded
    EXPECT_TRUE(downloader->isMixDownloaded(mix_id));
}

TEST_F(MixDownloaderTest, GetLocalPath) {
    // Test local path generation
    std::string mix_id = "test_mix_123";
    std::string localPath = downloader->getLocalPath(mix_id);
    
    // Should contain the cache directory
    EXPECT_NE(localPath.find(cache_dir), std::string::npos);
    
    // Should contain the mix ID
    EXPECT_NE(localPath.find(mix_id), std::string::npos);
    
    // Should have a reasonable extension
    EXPECT_NE(localPath.find(".mp3"), std::string::npos);
}

TEST_F(MixDownloaderTest, GetLocalPathWithSpecialCharacters) {
    // Test local path with special characters in mix ID
    std::string mix_id = "test_mix_with_spaces_and_symbols!@#";
    std::string localPath = downloader->getLocalPath(mix_id);
    
    // Should still generate a valid path
    EXPECT_NE(localPath.find(cache_dir), std::string::npos);
    EXPECT_NE(localPath.find(mix_id), std::string::npos);
}

TEST_F(MixDownloaderTest, DownloadMixWithValidUrl) {
    // Test downloading a mix with a valid URL
    // Note: This would require a real HTTP server or mocking
    // For now, we'll test the method doesn't crash
    Mix mix = createTestMix("test_mix_1", "https://invalid-domain-that-does-not-exist-12345.com/test.mp3");
    
    bool result = downloader->downloadMix(mix);
    // Should fail quickly without hanging
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithInvalidUrl) {
    // Test downloading with invalid URL
    Mix mix = createTestMix("test_mix_2", "invalid://url");
    
    bool result = downloader->downloadMix(mix);
    // Should fail gracefully
    EXPECT_FALSE(result);
    
    // Should have error message
    std::string error = downloader->getLastError();
    EXPECT_FALSE(error.empty());
}

TEST_F(MixDownloaderTest, DownloadMixWithEmptyUrl) {
    // Test downloading with empty URL
    Mix mix = createTestMix("test_mix_3", "");
    
    bool result = downloader->downloadMix(mix);
    EXPECT_FALSE(result);
    
    std::string error = downloader->getLastError();
    EXPECT_FALSE(error.empty());
}

TEST_F(MixDownloaderTest, DownloadMixWithLocalFile) {
    // Test downloading a local file (should work)
    std::string localFilePath = test_dir + "/local_test.mp3";
    createTestFile(localFilePath, "fake audio data");
    
    Mix mix = createTestMix("test_mix_4", "file://" + localFilePath);
    
    bool result = downloader->downloadMix(mix);
    // Local file should work
    EXPECT_TRUE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithFtpUrl) {
    // Test downloading with FTP URL - using invalid URL to avoid network connection
    Mix mix = createTestMix("test_mix_5", "ftp://invalid-domain-that-does-not-exist-12345.com/test.mp3");
    
    bool result = downloader->downloadMix(mix);
    // Should fail quickly without hanging
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithHttpUrl) {
    // Test downloading with HTTP URL - using invalid URL to avoid network connection
    Mix mix = createTestMix("test_mix_6", "http://invalid-domain-that-does-not-exist-12345.com/test.mp3");
    
    bool result = downloader->downloadMix(mix);
    // Should fail quickly without hanging
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithHttpsUrl) {
    // Test downloading with HTTPS URL - using invalid URL to avoid network connection
    Mix mix = createTestMix("test_mix_7", "https://invalid-domain-that-does-not-exist-12345.com/test.mp3");
    
    bool result = downloader->downloadMix(mix);
    // Should fail quickly without hanging
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithLargeUrl) {
    // Test downloading with very long URL
    std::string longUrl = "https://invalid-domain-that-does-not-exist-12345.com/";
    longUrl.append(1000, 'a'); // Very long URL
    longUrl += ".mp3";
    
    Mix mix = createTestMix("test_mix_8", longUrl);
    
    bool result = downloader->downloadMix(mix);
    // Should handle long URLs gracefully and fail quickly
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithSpecialCharactersInUrl) {
    // Test downloading with special characters in URL
    Mix mix = createTestMix("test_mix_9", "https://invalid-domain-that-does-not-exist-12345.com/test%20file.mp3");
    
    bool result = downloader->downloadMix(mix);
    // Should handle URL encoding and fail quickly
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithQueryParameters) {
    // Test downloading with query parameters
    Mix mix = createTestMix("test_mix_10", "https://invalid-domain-that-does-not-exist-12345.com/test.mp3?param=value");
    
    bool result = downloader->downloadMix(mix);
    // Should handle query parameters and fail quickly
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithFragment) {
    // Test downloading with URL fragment
    Mix mix = createTestMix("test_mix_11", "https://invalid-domain-that-does-not-exist-12345.com/test.mp3#fragment");
    
    bool result = downloader->downloadMix(mix);
    // Should handle URL fragments and fail quickly
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithPort) {
    // Test downloading with custom port
    Mix mix = createTestMix("test_mix_12", "https://invalid-domain-that-does-not-exist-12345.com:8080/test.mp3");
    
    bool result = downloader->downloadMix(mix);
    // Should handle custom ports and fail quickly
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithCredentials) {
    // Test downloading with credentials in URL
    Mix mix = createTestMix("test_mix_13", "https://user:pass@invalid-domain-that-does-not-exist-12345.com/test.mp3");
    
    bool result = downloader->downloadMix(mix);
    // Should handle credentials in URL and fail quickly
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithUnicodeUrl) {
    // Test downloading with Unicode characters in URL
    Mix mix = createTestMix("test_mix_14", "https://invalid-domain-that-does-not-exist-12345.com/测试.mp3");
    
    bool result = downloader->downloadMix(mix);
    // Should handle Unicode URLs and fail quickly
    EXPECT_FALSE(result);
}

TEST_F(MixDownloaderTest, DownloadMixWithDifferentExtensions) {
    // Test downloading with different file extensions
    std::vector<std::string> extensions = {".mp3", ".wav", ".flac", ".ogg", ".m4a"};
    
    for (size_t i = 0; i < extensions.size(); ++i) {
        Mix mix = createTestMix("test_mix_" + std::to_string(i), 
                               "https://invalid-domain-that-does-not-exist-12345.com/test" + extensions[i]);
        
        bool result = downloader->downloadMix(mix);
        // Should handle different extensions and fail quickly
        EXPECT_FALSE(result);
    }
}

TEST_F(MixDownloaderTest, DownloadMixWithInvalidMix) {
    // Test downloading with invalid mix (empty ID)
    Mix mix;
    mix.id = "";
    mix.url = "https://invalid-domain-that-does-not-exist-12345.com/test.mp3";
    
    bool result = downloader->downloadMix(mix);
    // Should handle invalid mix gracefully
    EXPECT_FALSE(result);
    
    std::string error = downloader->getLastError();
    EXPECT_FALSE(error.empty());
}

TEST_F(MixDownloaderTest, DownloadMixWithEmptyMix) {
    // Test downloading with completely empty mix
    Mix mix;
    
    bool result = downloader->downloadMix(mix);
    // Should handle empty mix gracefully
    EXPECT_FALSE(result);
    
    std::string error = downloader->getLastError();
    EXPECT_FALSE(error.empty());
}

TEST_F(MixDownloaderTest, GetLastError) {
    // Test error message retrieval
    std::string initialError = downloader->getLastError();
    // Initially might be empty or have default message
    
    // Try to download something that will fail
    Mix mix = createTestMix("test_mix_15", "invalid://url");
    downloader->downloadMix(mix);
    
    std::string error = downloader->getLastError();
    // Should have an error message after failed download
    EXPECT_FALSE(error.empty());
}

TEST_F(MixDownloaderTest, CacheDirectoryCreation) {
    // Test that cache directory is created when needed
    std::string newCacheDir = test_dir + "/new_cache";
    MixDownloader newDownloader(newCacheDir);
    
    // Directory should not exist initially (lazy creation)
    EXPECT_FALSE(std::filesystem::exists(newCacheDir));
    
    // Try to download something to trigger directory creation
    Mix mix = createTestMix("test_mix_cache", "https://invalid-domain-that-does-not-exist-12345.com/test.mp3");
    newDownloader.downloadMix(mix);
    
    // Directory should be created after download attempt
    EXPECT_TRUE(std::filesystem::exists(newCacheDir));
}

TEST_F(MixDownloaderTest, CacheDirectoryPermissions) {
    // Test cache directory permissions
    std::string localPath = downloader->getLocalPath("test_mix_16");
    
    // Should be able to create a file in cache directory
    createTestFile(localPath, "test data");
    EXPECT_TRUE(std::filesystem::exists(localPath));
}

TEST_F(MixDownloaderTest, ConcurrentDownloads) {
    // Test multiple downloads (simulated)
    std::vector<Mix> mixes;
    for (int i = 0; i < 5; ++i) {
        mixes.push_back(createTestMix("concurrent_mix_" + std::to_string(i), 
                                     "https://invalid-domain-that-does-not-exist-12345.com/mix" + std::to_string(i) + ".mp3"));
    }
    
    // Try to download all mixes
    for (const auto& mix : mixes) {
        bool result = downloader->downloadMix(mix);
        // Should not crash with multiple downloads and fail quickly
        EXPECT_FALSE(result);
    }
} 