#include "url_utils.hpp"

#include <gtest/gtest.h>

using AutoVibez::Utils::UrlUtils;

class UrlUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(UrlUtilsTest, ExtractFilenameFromUrl_ValidUrls) {
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com/song.mp3"), "song.mp3");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("http://music.com/artist/track.mp3"), "track.mp3");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("ftp://files.com/music/song.mp3"), "song.mp3");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com/path/to/file.mp3"), "file.mp3");
}

TEST_F(UrlUtilsTest, ExtractFilenameFromUrl_UrlEncoded) {
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com/song%20with%20spaces.mp3"), "song with spaces.mp3");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com/file%2Bplus.mp3"), "file+plus.mp3");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com/song%2Bwith%2Bplus.mp3"), "song+with+plus.mp3");
}

TEST_F(UrlUtilsTest, ExtractFilenameFromUrl_InvalidUrls) {
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl(""), "");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com"), "example.com");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("not-a-url"), "");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com/"), "");
}

TEST_F(UrlUtilsTest, UrlDecode_ValidEncoded) {
    EXPECT_EQ(UrlUtils::urlDecode("Hello%20World"), "Hello World");
    EXPECT_EQ(UrlUtils::urlDecode("file%2Bplus"), "file+plus");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com/song%20with%20spaces.mp3"), "song with spaces.mp3");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com/file%2Bplus.mp3"), "file+plus.mp3");
}

TEST_F(UrlUtilsTest, UrlDecode_InvalidEncoded) {
    EXPECT_EQ(UrlUtils::urlDecode("Hello%2"), "Hello%2");    // Invalid hex
    EXPECT_EQ(UrlUtils::urlDecode("Hello%2G"), "Hello%2G");  // Invalid hex
    EXPECT_EQ(UrlUtils::urlDecode("Hello%"), "Hello%");      // Incomplete
}

TEST_F(UrlUtilsTest, GetUrlExtension_ValidUrls) {
    EXPECT_EQ(UrlUtils::getUrlExtension("https://example.com/song.mp3"), "mp3");
    EXPECT_EQ(UrlUtils::getUrlExtension("http://music.com/artist/track.MP3"), "mp3");
    EXPECT_EQ(UrlUtils::extractFilenameFromUrl("https://example.com/file.wav"), "file.wav");
    EXPECT_EQ(UrlUtils::getUrlExtension("https://example.com/file.wav"), "wav");
}

TEST_F(UrlUtilsTest, GetUrlExtension_NoExtension) {
    EXPECT_EQ(UrlUtils::getUrlExtension("https://example.com/song"), "");
    EXPECT_EQ(UrlUtils::getUrlExtension("https://example.com/"), "");
    EXPECT_EQ(UrlUtils::getUrlExtension(""), "");
}

TEST_F(UrlUtilsTest, IsValidUrl_ValidUrls) {
    EXPECT_TRUE(UrlUtils::isValidUrl("https://example.com"));
    EXPECT_TRUE(UrlUtils::isValidUrl("http://music.com/song.mp3"));
    EXPECT_TRUE(UrlUtils::isValidUrl("ftp://files.com/music.mp3"));
    EXPECT_TRUE(UrlUtils::isValidUrl("https://example.com/path/to/file.mp3"));
}

TEST_F(UrlUtilsTest, IsValidUrl_InvalidUrls) {
    EXPECT_FALSE(UrlUtils::isValidUrl(""));
    EXPECT_FALSE(UrlUtils::isValidUrl("not-a-url"));
    EXPECT_FALSE(UrlUtils::isValidUrl("file://local/path"));
    EXPECT_FALSE(UrlUtils::isValidUrl("https://"));
    EXPECT_FALSE(UrlUtils::isValidUrl("http://"));
}

TEST_F(UrlUtilsTest, GetDomain_ValidUrls) {
    EXPECT_EQ(UrlUtils::getDomain("https://example.com/song.mp3"), "example.com");
    EXPECT_EQ(UrlUtils::getDomain("http://music.com/artist/track.mp3"), "music.com");
    EXPECT_EQ(UrlUtils::getDomain("https://sub.example.com/file.mp3"), "sub.example.com");
    EXPECT_EQ(UrlUtils::getDomain("https://example.com"), "example.com");
}

TEST_F(UrlUtilsTest, GetDomain_InvalidUrls) {
    EXPECT_EQ(UrlUtils::getDomain(""), "");
    EXPECT_EQ(UrlUtils::getDomain("not-a-url"), "");
    EXPECT_EQ(UrlUtils::getDomain("https://"), "");
}

TEST_F(UrlUtilsTest, GetProtocol_ValidUrls) {
    EXPECT_EQ(UrlUtils::getProtocol("https://example.com/song.mp3"), "https");
    EXPECT_EQ(UrlUtils::getProtocol("http://music.com/artist/track.mp3"), "http");
    EXPECT_EQ(UrlUtils::getProtocol("ftp://files.com/music.mp3"), "ftp");
}

TEST_F(UrlUtilsTest, GetProtocol_InvalidUrls) {
    EXPECT_EQ(UrlUtils::getProtocol(""), "");
    EXPECT_EQ(UrlUtils::getProtocol("not-a-url"), "");
    EXPECT_EQ(UrlUtils::getProtocol("example.com"), "");
}

TEST_F(UrlUtilsTest, ComplexUrlHandling) {
    std::string complex_url = "https://music.example.com/artist/album/song%20with%20spaces.mp3?param=value#fragment";

    EXPECT_EQ(UrlUtils::extractFilenameFromUrl(complex_url), "song with spaces.mp3");
    EXPECT_EQ(UrlUtils::getUrlExtension(complex_url), "mp3");
    EXPECT_EQ(UrlUtils::getDomain(complex_url), "music.example.com");
    EXPECT_EQ(UrlUtils::getProtocol(complex_url), "https");
    EXPECT_TRUE(UrlUtils::isValidUrl(complex_url));
}
