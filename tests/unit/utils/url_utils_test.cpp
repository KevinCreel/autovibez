#include <gtest/gtest.h>
#include "utils/url_utils.hpp"

TEST(UrlUtilsTest, ExtractFilenameFromUrl) {
    // Test basic filename extraction
    EXPECT_EQ(AutoVibez::Utils::UrlUtils::extractFilenameFromUrl("https://example.com/mix.mp3"), "mix.mp3");
    EXPECT_EQ(AutoVibez::Utils::UrlUtils::extractFilenameFromUrl("https://example.com/path/to/file.yaml"), "file.yaml");
    
    // Test with query parameters
    EXPECT_EQ(AutoVibez::Utils::UrlUtils::extractFilenameFromUrl("https://example.com/mix.mp3?param=value"), "mix.mp3");
    
    // Test edge cases
    EXPECT_EQ(AutoVibez::Utils::UrlUtils::extractFilenameFromUrl("https://example.com/"), "");
    EXPECT_EQ(AutoVibez::Utils::UrlUtils::extractFilenameFromUrl(""), "");
}

TEST(UrlUtilsTest, IsValidUrl) {
    // Test valid URLs
    EXPECT_TRUE(AutoVibez::Utils::UrlUtils::isValidUrl("https://example.com"));
    EXPECT_TRUE(AutoVibez::Utils::UrlUtils::isValidUrl("http://example.com/file.mp3"));
    EXPECT_TRUE(AutoVibez::Utils::UrlUtils::isValidUrl("https://example.com/path/to/file.yaml"));
    
    // Test invalid URLs
    EXPECT_FALSE(AutoVibez::Utils::UrlUtils::isValidUrl("not-a-url"));
    EXPECT_FALSE(AutoVibez::Utils::UrlUtils::isValidUrl(""));
    
    // Test other protocols (regex allows http, https, ftp)
    EXPECT_TRUE(AutoVibez::Utils::UrlUtils::isValidUrl("ftp://example.com"));
    EXPECT_TRUE(AutoVibez::Utils::UrlUtils::isValidUrl("http://example.com"));
    EXPECT_TRUE(AutoVibez::Utils::UrlUtils::isValidUrl("https://example.com"));
}
