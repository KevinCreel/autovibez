#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "data/mix_downloader.hpp"
#include "data/mix_metadata.hpp"
#include "audio/mp3_analyzer.hpp"
#include "utils/constants.hpp"
#include "utils/url_utils.hpp" // Added for URL validation tests

class MixDownloaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directories
        test_dir = std::filesystem::temp_directory_path() / "mix_downloader_test";
        mixes_dir = test_dir / "mixes";
        std::filesystem::create_directories(test_dir);
        std::filesystem::create_directories(mixes_dir);
    }

    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(test_dir);
    }

    std::string createMockMP3File(const std::string& filename, 
                                  const std::string& content = "mock mp3 content") {
        std::string filepath = (test_dir / filename).string();
        std::ofstream file(filepath);
        
        // Create a file that meets minimum MP3 size requirement
        std::string large_content = content;
        while (large_content.length() < Constants::MIN_MP3_FILE_SIZE) {
            large_content += content;
        }
        
        file << large_content;
        file.close();
        return filepath;
    }

    AutoVibez::Data::Mix createMockMix(const std::string& id, const std::string& url, 
                      const std::string& original_filename = "") {
        AutoVibez::Data::Mix mix;
        mix.id = id;
        mix.url = url;
        mix.original_filename = original_filename;
        return mix;
    }

    std::filesystem::path test_dir;
    std::filesystem::path mixes_dir;
};

TEST_F(MixDownloaderTest, ConstructorInitialization) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    // Should be initialized without error
    EXPECT_TRUE(downloader.getLastError().empty());
}

TEST_F(MixDownloaderTest, DownloadMixWithEmptyUrl) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", "");
    bool result = downloader.downloadMix(mix);
    
    EXPECT_FALSE(result);
    EXPECT_FALSE(downloader.getLastError().empty());
    EXPECT_TRUE(downloader.getLastError().find("Mix URL is empty") != std::string::npos);
}

TEST_F(MixDownloaderTest, DownloadMixWithInvalidUrl) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", "invalid://url");
    bool result = downloader.downloadMix(mix);
    
    // Should fail with invalid URL
    EXPECT_FALSE(result);
    EXPECT_FALSE(downloader.getLastError().empty());
}

TEST_F(MixDownloaderTest, DownloadLocalFile) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    // Create a mock MP3 file
    std::string source_file = createMockMP3File("source.mp3");
    std::string file_url = "file://" + source_file;
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", file_url);
    bool result = downloader.downloadMix(mix);
    
    // Should succeed for local file
    EXPECT_TRUE(result);
    EXPECT_TRUE(downloader.getLastError().empty());
    
    // Should create local copy
    std::string local_path = downloader.getLocalPath("test_id");
    EXPECT_TRUE(std::filesystem::exists(local_path));
}

TEST_F(MixDownloaderTest, DownloadLocalFileWithOriginalFilename) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    // Create a mock MP3 file
    std::string source_file = createMockMP3File("source.mp3");
    std::string file_url = "file://" + source_file;
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", file_url, "original_name.mp3");
    bool result = downloader.downloadMix(mix);
    
    // Should succeed for local file
    EXPECT_TRUE(result);
    EXPECT_TRUE(downloader.getLastError().empty());
    
    // Should use original filename
    std::string local_path = downloader.getLocalPathWithOriginalFilename(mix);
    EXPECT_TRUE(std::filesystem::exists(local_path));
}

TEST_F(MixDownloaderTest, IsMixDownloaded) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    // Initially should not be downloaded
    EXPECT_FALSE(downloader.isMixDownloaded("test_id"));
    
    // Create a mock file
    std::string local_path = downloader.getLocalPath("test_id");
    std::ofstream file(local_path);
    file << "mock content";
    file.close();
    
    // Should now be considered downloaded
    EXPECT_TRUE(downloader.isMixDownloaded("test_id"));
}

TEST_F(MixDownloaderTest, GetLocalPath) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    std::string local_path = downloader.getLocalPath("test_id");
    
    // Should return expected path
    EXPECT_TRUE(local_path.find("test_id") != std::string::npos);
    EXPECT_TRUE(local_path.find(".mp3") != std::string::npos);
    EXPECT_TRUE(local_path.find(mixes_path) != std::string::npos);
}

TEST_F(MixDownloaderTest, GetTemporaryPath) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    std::string temp_path = downloader.getTemporaryPath("test_id");
    
    // Should return expected temporary path
    EXPECT_TRUE(temp_path.find("test_id") != std::string::npos);
    EXPECT_TRUE(temp_path.find(".tmp") != std::string::npos);
    EXPECT_TRUE(temp_path.find(mixes_path) != std::string::npos);
}

TEST_F(MixDownloaderTest, GetLocalPathWithOriginalFilename) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", "http://invalid-domain-that-doesnt-exist.com/mix.mp3", "original_name.mp3");
    std::string local_path = downloader.getLocalPathWithOriginalFilename(mix);
    
    // Should use original filename
    EXPECT_TRUE(local_path.find("original_name.mp3") != std::string::npos);
    EXPECT_TRUE(local_path.find(mixes_path) != std::string::npos);
}

TEST_F(MixDownloaderTest, GetLocalPathWithOriginalFilenameEmpty) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", "http://invalid-domain-that-doesnt-exist.com/mix.mp3", "");
    std::string local_path = downloader.getLocalPathWithOriginalFilename(mix);
    
    // Should fall back to default path
    EXPECT_TRUE(local_path.find("test_id") != std::string::npos);
    EXPECT_TRUE(local_path.find(".mp3") != std::string::npos);
}

TEST_F(MixDownloaderTest, DownloadMixWithTitleNamingNullAnalyzer) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", "http://invalid-domain-that-doesnt-exist.com/mix.mp3");
    bool result = downloader.downloadMixWithTitleNaming(mix, nullptr);
    
    EXPECT_FALSE(result);
    EXPECT_FALSE(downloader.getLastError().empty());
    EXPECT_TRUE(downloader.getLastError().find("MP3Analyzer is required") != std::string::npos);
}

TEST_F(MixDownloaderTest, DownloadMixWithTitleNamingEmptyUrl) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", "");
    AutoVibez::Audio::MP3Analyzer analyzer;
    bool result = downloader.downloadMixWithTitleNaming(mix, &analyzer);
    
    EXPECT_FALSE(result);
    EXPECT_FALSE(downloader.getLastError().empty());
    EXPECT_TRUE(downloader.getLastError().find("URL is empty") != std::string::npos);
}

TEST_F(MixDownloaderTest, DownloadMixWithTitleNamingLocalFile) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    // Create a mock MP3 file
    std::string source_file = createMockMP3File("source.mp3");
    std::string file_url = "file://" + source_file;
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", file_url);
    AutoVibez::Audio::MP3Analyzer analyzer;
    bool result = downloader.downloadMixWithTitleNaming(mix, &analyzer);
    
    // Should succeed for local file
    EXPECT_TRUE(result);
    EXPECT_TRUE(downloader.getLastError().empty());
    
    // Should create local copy
    std::string local_path = downloader.getLocalPath("test_id");
    EXPECT_TRUE(std::filesystem::exists(local_path));
}

TEST_F(MixDownloaderTest, MultipleDownloaderInstances) {
    std::string mixes_path = mixes_dir.string();
    
    // Create multiple downloader instances
    AutoVibez::Data::MixDownloader downloader1(mixes_path);
    AutoVibez::Data::MixDownloader downloader2(mixes_path);
    
    // Both should work independently
    AutoVibez::Data::Mix mix = createMockMix("test_id", "http://invalid-domain-that-doesnt-exist.com/mix.mp3");
    
    bool result1 = downloader1.downloadMix(mix);
    bool result2 = downloader2.downloadMix(mix);
    
    // Both should fail with invalid URLs
    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
    EXPECT_FALSE(downloader1.getLastError().empty());
    EXPECT_FALSE(downloader2.getLastError().empty());
}

TEST_F(MixDownloaderTest, ErrorStateManagement) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    // Initially should have no error
    EXPECT_TRUE(downloader.getLastError().empty());
    
    // Try to download with empty URL
    AutoVibez::Data::Mix mix = createMockMix("test_id", "");
    downloader.downloadMix(mix);
    
    // Should have error
    EXPECT_FALSE(downloader.getLastError().empty());
}

TEST_F(MixDownloaderTest, FilePathWithSpecialCharacters) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", "http://invalid-domain-that-doesnt-exist.com/mix.mp3", "song with spaces & symbols.mp3");
    std::string local_path = downloader.getLocalPathWithOriginalFilename(mix);
    
    // Should handle special characters in filename
    EXPECT_TRUE(local_path.find("song with spaces & symbols.mp3") != std::string::npos);
}

TEST_F(MixDownloaderTest, UnicodeFilePath) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", "http://invalid-domain-that-doesnt-exist.com/mix.mp3", "sóng_ñáme.mp3");
    std::string local_path = downloader.getLocalPathWithOriginalFilename(mix);
    
    // Should handle unicode characters in filename
    EXPECT_TRUE(local_path.find("sóng_ñáme.mp3") != std::string::npos);
}

TEST_F(MixDownloaderTest, RepeatedDownloadAttempts) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    AutoVibez::Data::Mix mix = createMockMix("test_id", "http://invalid-domain-that-doesnt-exist.com/mix.mp3");
    
    // Try to download the same mix multiple times
    for (int i = 0; i < 3; i++) {
        bool result = downloader.downloadMix(mix);
        EXPECT_FALSE(result);
        EXPECT_FALSE(downloader.getLastError().empty());
    }
}

TEST_F(MixDownloaderTest, ValidUrlFormatValidation) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    // Test various valid URL formats that should pass URL validation
    // Use domains that definitely don't exist to avoid real network calls
    std::vector<std::string> valid_urls = {
        "http://invalid-domain-that-doesnt-exist-12345.com/mix.mp3",
        "https://another-fake-domain-67890.net/mix.mp3", 
        "http://fake-subdomain.invalid-domain-that-doesnt-exist-12345.com/path/to/mix.mp3",
        "https://fake-domain-67890.net:8080/mix.mp3",
        "http://fake-domain-67890.net/mix.mp3?param=value",
        "https://fake-domain-67890.net/mix.mp3#fragment",
        "ftp://fake-domain-67890.net/mix.mp3"  // FTP is supported by our regex
    };
    
    for (const auto& url : valid_urls) {
        AutoVibez::Data::Mix mix = createMockMix("test_id", url);
        
        // The URL should be considered valid by our validation
        EXPECT_TRUE(AutoVibez::Utils::UrlUtils::isValidUrl(url));
        
        // The downloader should attempt the download (but fail due to network)
        bool result = downloader.downloadMix(mix);
        // We expect this to fail because we're not making real network calls
        // but the URL format validation should pass
        EXPECT_FALSE(result);
        // Error should be about network/download failure, not URL format
        EXPECT_FALSE(downloader.getLastError().empty());
        EXPECT_TRUE(downloader.getLastError().find("URL") == std::string::npos);
    }
}

TEST_F(MixDownloaderTest, InvalidUrlFormatValidation) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    // Test various invalid URL formats that should fail URL validation
    // Our regex pattern is: (https?|ftp)://[^\s/$.?#].[^\s]*
    // This is quite permissive, so we need to test URLs that definitely don't match
    std::vector<std::string> invalid_urls = {
        "invalid://url",              // Invalid protocol
        "not-a-url",                  // No protocol
        "http://",                    // Incomplete URL (no domain)
        "https://",                   // Incomplete URL (no domain)
        "://example.com/mix.mp3",    // Missing protocol
        "http:// example.com/mix.mp3", // Contains space (invalid character)
        "http://example.com /mix.mp3", // Contains space (invalid character)
        "http://example.com/mix.mp3 ", // Contains space (invalid character)
        "http://example.com/mix.mp3\n", // Contains newline (invalid character)
        "http://example.com/mix.mp3\t"  // Contains tab (invalid character)
    };
    
    for (const auto& url : invalid_urls) {
        AutoVibez::Data::Mix mix = createMockMix("test_id", url);
        
        // The URL should be considered invalid by our validation
        bool is_valid = AutoVibez::Utils::UrlUtils::isValidUrl(url);
        EXPECT_FALSE(is_valid) << "URL should be invalid: " << url;
        
        // The downloader should reject invalid URLs immediately
        bool result = downloader.downloadMix(mix);
        EXPECT_FALSE(result);
        EXPECT_FALSE(downloader.getLastError().empty());
        // Error should mention URL format
        EXPECT_TRUE(downloader.getLastError().find("URL") != std::string::npos || 
                   downloader.getLastError().find("Invalid") != std::string::npos);
    }
}

TEST_F(MixDownloaderTest, UrlValidationEdgeCases) {
    std::string mixes_path = mixes_dir.string();
    AutoVibez::Data::MixDownloader downloader(mixes_path);
    
    // Test edge cases for URL validation
    std::vector<std::pair<std::string, bool>> edge_cases = {
        {"http://localhost/mix.mp3", true},           // Localhost should be valid
        {"https://127.0.0.1/mix.mp3", true},         // IP address should be valid
        {"http://[::1]/mix.mp3", true},               // IPv6 should be valid
        {"http://example.com:80/mix.mp3", true},      // Standard port
        {"http://example.com:443/mix.mp3", true},     // Non-standard port
        {"http://example.com/mix.mp3", true},         // Standard HTTP
        {"https://example.com/mix.mp3", true},        // Standard HTTPS
        {"ftp://example.com/mix.mp3", true},          // FTP is supported
        {"http://example.com/mix.mp3/", true},        // Trailing slash
        {"http://example.com/mix.mp3#", true},        // Empty fragment
        {"http://example.com/mix.mp3?", true},        // Empty query
        // Note: Case sensitivity depends on the regex pattern
        // Our current regex is case-sensitive, so these should be false:
        {"HTTP://EXAMPLE.COM/MIX.MP3", false},        // Case sensitive
        {"HTTPS://EXAMPLE.COM/MIX.MP3", false},       // Case sensitive
    };
    
    for (const auto& [url, should_be_valid] : edge_cases) {
        AutoVibez::Data::Mix mix = createMockMix("test_id", url);
        
        bool is_valid = AutoVibez::Utils::UrlUtils::isValidUrl(url);
        EXPECT_EQ(is_valid, should_be_valid) << "URL: " << url;
        
        if (should_be_valid) {
            // Valid URL format should pass validation
            EXPECT_TRUE(is_valid);
        } else {
            // Invalid URL format should fail validation
            EXPECT_FALSE(is_valid);
        }
    }
}
