#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "string_utils.hpp"

using namespace StringUtils;

class StringUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed
    }
    
    void TearDown() override {
        // No cleanup needed
    }
};

TEST_F(StringUtilsTest, ToTitleCase) {
    // Test basic title case conversion
    EXPECT_EQ(toTitleCase("hello world"), "Hello World");
    EXPECT_EQ(toTitleCase("HELLO WORLD"), "Hello World");
    EXPECT_EQ(toTitleCase("hello-world"), "Hello-World");
    EXPECT_EQ(toTitleCase("hello_world"), "Hello_world"); // Only spaces and hyphens trigger capitalization
}

TEST_F(StringUtilsTest, ToTitleCaseWithSingleWord) {
    // Test single word title case
    EXPECT_EQ(toTitleCase("hello"), "Hello");
    EXPECT_EQ(toTitleCase("WORLD"), "World");
    EXPECT_EQ(toTitleCase("a"), "A");
    EXPECT_EQ(toTitleCase("Z"), "Z");
}

TEST_F(StringUtilsTest, ToTitleCaseWithMixedCase) {
    // Test mixed case input
    EXPECT_EQ(toTitleCase("hElLo WoRlD"), "Hello World");
    EXPECT_EQ(toTitleCase("MiXeD cAsE"), "Mixed Case");
    EXPECT_EQ(toTitleCase("tEsT"), "Test");
}

TEST_F(StringUtilsTest, ToTitleCaseWithSpecialCharacters) {
    // Test with special characters
    EXPECT_EQ(toTitleCase("hello!world"), "Hello!world"); // Only spaces and hyphens trigger capitalization
    EXPECT_EQ(toTitleCase("hello@world"), "Hello@world");
    EXPECT_EQ(toTitleCase("hello#world"), "Hello#world");
    EXPECT_EQ(toTitleCase("hello$world"), "Hello$world");
}

TEST_F(StringUtilsTest, ToTitleCaseWithNumbers) {
    // Test with numbers
    EXPECT_EQ(toTitleCase("hello123world"), "Hello123world"); // Only spaces and hyphens trigger capitalization
    EXPECT_EQ(toTitleCase("123hello456world"), "123Hello456world");
    EXPECT_EQ(toTitleCase("hello 123 world"), "Hello 123 World");
}

TEST_F(StringUtilsTest, ToTitleCaseWithEmptyString) {
    // Test empty string
    EXPECT_EQ(toTitleCase(""), "");
}

TEST_F(StringUtilsTest, ToTitleCaseWithWhitespace) {
    // Test with various whitespace
    EXPECT_EQ(toTitleCase("  hello  world  "), "  Hello  World  ");
    EXPECT_EQ(toTitleCase("\thello\nworld\r"), "\tHello\nWorld\r");
    EXPECT_EQ(toTitleCase("  a  b  c  "), "  A  B  C  ");
}

TEST_F(StringUtilsTest, ToTitleCaseWithUnicode) {
    // Test with Unicode characters
    EXPECT_EQ(toTitleCase("café world"), "Café World");
    EXPECT_EQ(toTitleCase("naïve test"), "Naïve Test");
    EXPECT_EQ(toTitleCase("résumé"), "Résumé");
}

TEST_F(StringUtilsTest, ExtractFileName) {
    // Test basic filename extraction
    EXPECT_EQ(extractFileName("/path/to/file.txt"), "file.txt");
    EXPECT_EQ(extractFileName("file.txt"), "file.txt");
    EXPECT_EQ(extractFileName("/file.txt"), "file.txt");
    EXPECT_EQ(extractFileName("file"), "file");
}

TEST_F(StringUtilsTest, ExtractFileNameWithComplexPaths) {
    // Test complex path scenarios
    EXPECT_EQ(extractFileName("/home/user/documents/file.txt"), "file.txt");
    EXPECT_EQ(extractFileName("C:\\Users\\User\\Documents\\file.txt"), "C:\\Users\\User\\Documents\\file.txt"); // Only handles forward slashes
    EXPECT_EQ(extractFileName("/path/with/multiple/directories/file.mp3"), "file.mp3");
}

TEST_F(StringUtilsTest, ExtractFileNameWithSpecialCharacters) {
    // Test filenames with special characters
    EXPECT_EQ(extractFileName("/path/to/file with spaces.txt"), "file with spaces.txt");
    EXPECT_EQ(extractFileName("/path/to/file-with-dashes.txt"), "file-with-dashes.txt");
    EXPECT_EQ(extractFileName("/path/to/file_with_underscores.txt"), "file_with_underscores.txt");
}

TEST_F(StringUtilsTest, ExtractFileNameWithNoPath) {
    // Test when no path is present
    EXPECT_EQ(extractFileName("filename.txt"), "filename.txt");
    EXPECT_EQ(extractFileName("file"), "file");
    EXPECT_EQ(extractFileName(""), "");
}

TEST_F(StringUtilsTest, ExtractFileNameWithTrailingSlash) {
    // Test with trailing slash
    EXPECT_EQ(extractFileName("/path/to/"), "");
    EXPECT_EQ(extractFileName("/"), "");
    EXPECT_EQ(extractFileName("path/to/"), "");
}

TEST_F(StringUtilsTest, ExtractFileNameWithMultipleExtensions) {
    // Test with multiple dots
    EXPECT_EQ(extractFileName("/path/to/file.backup.txt"), "file.backup.txt");
    EXPECT_EQ(extractFileName("/path/to/file.name.with.dots"), "file.name.with.dots");
}

TEST_F(StringUtilsTest, FormatPresetName) {
    // Test preset name formatting
    EXPECT_EQ(formatPresetName("/path/to/preset.milk"), "preset.milk");
    EXPECT_EQ(formatPresetName("preset.milk"), "preset.milk");
    EXPECT_EQ(formatPresetName("/preset.milk"), "preset.milk");
}

TEST_F(StringUtilsTest, FormatPresetNameWithComplexPaths) {
    // Test complex preset paths
    EXPECT_EQ(formatPresetName("/home/user/.projectm/presets/preset.milk"), "preset.milk");
    EXPECT_EQ(formatPresetName("C:\\Users\\User\\Documents\\presets\\preset.milk"), "C:\\Users\\User\\Documents\\presets\\preset.milk"); // Only handles forward slashes
}

TEST_F(StringUtilsTest, FormatPresetNameWithSpecialCharacters) {
    // Test preset names with special characters
    EXPECT_EQ(formatPresetName("/path/to/preset with spaces.milk"), "preset with spaces.milk");
    EXPECT_EQ(formatPresetName("/path/to/preset-with-dashes.milk"), "preset-with-dashes.milk");
    EXPECT_EQ(formatPresetName("/path/to/preset_with_underscores.milk"), "preset_with_underscores.milk");
}

TEST_F(StringUtilsTest, FormatPresetNameWithNoPath) {
    // Test when no path is present
    EXPECT_EQ(formatPresetName("preset.milk"), "preset.milk");
    EXPECT_EQ(formatPresetName(""), "");
}

TEST_F(StringUtilsTest, Trim) {
    // Test basic trimming
    EXPECT_EQ(trim("  hello world  "), "hello world");
    EXPECT_EQ(trim("hello world"), "hello world");
    EXPECT_EQ(trim("  hello  world  "), "hello  world");
}

TEST_F(StringUtilsTest, TrimWithDifferentWhitespace) {
    // Test with different whitespace characters
    EXPECT_EQ(trim("\thello\nworld\r"), "hello\nworld");
    EXPECT_EQ(trim("\fhello\vworld"), "hello\vworld");
    EXPECT_EQ(trim(" \t\n\r\f\vhello world \t\n\r\f\v"), "hello world");
}

TEST_F(StringUtilsTest, TrimWithEmptyString) {
    // Test empty string
    EXPECT_EQ(trim(""), "");
    EXPECT_EQ(trim("   "), "");
    EXPECT_EQ(trim("\t\n\r\f\v"), "");
}

TEST_F(StringUtilsTest, TrimWithOnlyWhitespace) {
    // Test strings with only whitespace
    EXPECT_EQ(trim(" "), "");
    EXPECT_EQ(trim("\t"), "");
    EXPECT_EQ(trim("\n"), "");
    EXPECT_EQ(trim("\r"), "");
    EXPECT_EQ(trim("\f"), "");
    EXPECT_EQ(trim("\v"), "");
}

TEST_F(StringUtilsTest, TrimWithNoWhitespace) {
    // Test strings with no whitespace
    EXPECT_EQ(trim("hello"), "hello");
    EXPECT_EQ(trim("hello world"), "hello world");
    EXPECT_EQ(trim("123"), "123");
}

TEST_F(StringUtilsTest, StartsWith) {
    // Test basic startsWith functionality
    EXPECT_TRUE(startsWith("hello world", "hello"));
    EXPECT_TRUE(startsWith("hello world", ""));
    EXPECT_FALSE(startsWith("hello world", "world"));
    EXPECT_FALSE(startsWith("hello world", "hello world extra"));
}

TEST_F(StringUtilsTest, StartsWithCaseSensitive) {
    // Test case sensitivity
    EXPECT_TRUE(startsWith("Hello World", "Hello"));
    EXPECT_FALSE(startsWith("Hello World", "hello"));
    EXPECT_FALSE(startsWith("hello world", "Hello"));
}

TEST_F(StringUtilsTest, StartsWithEdgeCases) {
    // Test edge cases
    EXPECT_TRUE(startsWith("", ""));
    EXPECT_FALSE(startsWith("", "hello"));
    EXPECT_TRUE(startsWith("hello", "hello"));
    EXPECT_FALSE(startsWith("hello", "hello world"));
}

TEST_F(StringUtilsTest, StartsWithSpecialCharacters) {
    // Test with special characters
    EXPECT_TRUE(startsWith("hello-world", "hello-"));
    EXPECT_TRUE(startsWith("hello_world", "hello_"));
    EXPECT_TRUE(startsWith("hello!world", "hello!"));
    EXPECT_TRUE(startsWith("hello@world", "hello@"));
}

TEST_F(StringUtilsTest, StartsWithUnicode) {
    // Test with Unicode characters
    EXPECT_TRUE(startsWith("café world", "café"));
    EXPECT_TRUE(startsWith("naïve test", "naïve"));
    EXPECT_FALSE(startsWith("café world", "cafe"));
}

TEST_F(StringUtilsTest, EndsWith) {
    // Test basic endsWith functionality
    EXPECT_TRUE(endsWith("hello world", "world"));
    EXPECT_TRUE(endsWith("hello world", ""));
    EXPECT_FALSE(endsWith("hello world", "hello"));
    EXPECT_FALSE(endsWith("hello world", "extra hello world"));
}

TEST_F(StringUtilsTest, EndsWithCaseSensitive) {
    // Test case sensitivity
    EXPECT_TRUE(endsWith("Hello World", "World"));
    EXPECT_FALSE(endsWith("Hello World", "world"));
    EXPECT_FALSE(endsWith("hello world", "World"));
}

TEST_F(StringUtilsTest, EndsWithEdgeCases) {
    // Test edge cases
    EXPECT_TRUE(endsWith("", ""));
    EXPECT_FALSE(endsWith("", "world"));
    EXPECT_TRUE(endsWith("world", "world"));
    EXPECT_FALSE(endsWith("world", "hello world"));
}

TEST_F(StringUtilsTest, EndsWithSpecialCharacters) {
    // Test with special characters
    EXPECT_TRUE(endsWith("hello-world", "-world"));
    EXPECT_TRUE(endsWith("hello_world", "_world"));
    EXPECT_TRUE(endsWith("hello!world", "!world"));
    EXPECT_TRUE(endsWith("hello@world", "@world"));
}

TEST_F(StringUtilsTest, EndsWithUnicode) {
    // Test with Unicode characters
    EXPECT_TRUE(endsWith("hello café", "café"));
    EXPECT_TRUE(endsWith("test naïve", "naïve"));
    EXPECT_FALSE(endsWith("hello café", "cafe"));
}

TEST_F(StringUtilsTest, EndsWithFileExtensions) {
    // Test with file extensions
    EXPECT_TRUE(endsWith("file.txt", ".txt"));
    EXPECT_TRUE(endsWith("file.mp3", ".mp3"));
    EXPECT_TRUE(endsWith("file.milk", ".milk"));
    EXPECT_FALSE(endsWith("file.txt", ".mp3"));
}

TEST_F(StringUtilsTest, CombinedOperations) {
    // Test combining multiple string operations
    std::string input = "  Hello World  ";
    std::string trimmed = trim(input);
    std::string titleCased = toTitleCase(trimmed);
    
    EXPECT_EQ(trimmed, "Hello World");
    EXPECT_EQ(titleCased, "Hello World");
    
    EXPECT_TRUE(startsWith(titleCased, "Hello"));
    EXPECT_TRUE(endsWith(titleCased, "World"));
}

TEST_F(StringUtilsTest, PathOperations) {
    // Test path-related operations
    std::string path = "/path/to/file.txt";
    std::string filename = extractFileName(path);
    std::string formatted = formatPresetName(path);
    
    EXPECT_EQ(filename, "file.txt");
    EXPECT_EQ(formatted, "file.txt");
    
    EXPECT_TRUE(endsWith(filename, ".txt"));
    EXPECT_FALSE(startsWith(filename, "/"));
}

TEST_F(StringUtilsTest, UnicodeCombined) {
    // Test Unicode with combined operations
    std::string input = "  café world  ";
    std::string trimmed = trim(input);
    std::string titleCased = toTitleCase(trimmed);
    
    EXPECT_EQ(trimmed, "café world");
    EXPECT_EQ(titleCased, "Café World");
    
    EXPECT_TRUE(startsWith(titleCased, "Café"));
    EXPECT_TRUE(endsWith(titleCased, "World"));
}

TEST_F(StringUtilsTest, PerformanceEdgeCases) {
    // Test performance edge cases
    std::string longString(1000, 'a');
    longString += "hello";
    longString += std::string(1000, 'b');
    
    EXPECT_TRUE(startsWith(longString, std::string(1000, 'a')));
    EXPECT_TRUE(endsWith(longString, std::string(1000, 'b')));
    
    std::string trimmed = trim(longString);
    EXPECT_EQ(trimmed, std::string(1000, 'a') + "hello" + std::string(1000, 'b'));
}

TEST_F(StringUtilsTest, EmptyStringOperations) {
    // Test operations on empty strings
    EXPECT_EQ(toTitleCase(""), "");
    EXPECT_EQ(extractFileName(""), "");
    EXPECT_EQ(formatPresetName(""), "");
    EXPECT_EQ(trim(""), "");
    EXPECT_TRUE(startsWith("", ""));
    EXPECT_TRUE(endsWith("", ""));
}

TEST_F(StringUtilsTest, SingleCharacterOperations) {
    // Test operations on single characters
    EXPECT_EQ(toTitleCase("a"), "A");
    EXPECT_EQ(toTitleCase("Z"), "Z");
    EXPECT_EQ(extractFileName("a"), "a");
    EXPECT_EQ(formatPresetName("a"), "a");
    EXPECT_EQ(trim("a"), "a");
    EXPECT_TRUE(startsWith("a", "a"));
    EXPECT_TRUE(endsWith("a", "a"));
} 