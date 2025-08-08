#include "utils/uuid_utils.hpp"

#include <gtest/gtest.h>

#include <regex>

TEST(HashIdUtilsTest, GenerateIdFromUrl) {
    // Test that hash-based ID generation works
    std::string url1 = "https://example.com/mix1.mp3";
    std::string url2 = "https://example.com/mix2.mp3";

    auto id1 = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(url1);
    auto id2 = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(url2);

    // Should generate different IDs for different URLs
    EXPECT_NE(id1, id2);

    // Should generate same ID for same URL (deterministic)
    auto id1_again = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(url1);
    EXPECT_EQ(id1, id1_again);

    // Should generate valid UUID format (test behavior, not exact positions)
    EXPECT_EQ(id1.length(), 36);  // UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    EXPECT_TRUE(std::regex_match(id1, std::regex(R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})")));
    EXPECT_TRUE(std::regex_match(id2, std::regex(R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})")));
}

TEST(HashIdUtilsTest, GenerateIdFromUrlEdgeCases) {
    // Test edge cases
    std::string emptyUrl = "";
    std::string specialChars = "https://example.com/file with spaces & symbols.mp3";
    std::string longUrl = "https://very-long-domain-name.example.com/very/deep/path/to/file.mp3";

    // Should handle empty URL
    auto emptyId = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(emptyUrl);
    EXPECT_EQ(emptyId.length(), 36);
    EXPECT_TRUE(
        std::regex_match(emptyId, std::regex(R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})")));

    // Should handle special characters
    auto specialId = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(specialChars);
    EXPECT_EQ(specialId.length(), 36);
    EXPECT_TRUE(
        std::regex_match(specialId, std::regex(R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})")));

    // Should handle long URLs
    auto longId = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(longUrl);
    EXPECT_EQ(longId.length(), 36);
    EXPECT_TRUE(
        std::regex_match(longId, std::regex(R"([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})")));

    // Should be deterministic for same input
    auto emptyId2 = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(emptyUrl);
    EXPECT_EQ(emptyId, emptyId2);
}

TEST(HashIdUtilsTest, HashBasedBehavior) {
    // Test that the implementation behaves like a hash-based ID system
    std::string url1 = "https://example.com/mix1.mp3";
    std::string url2 = "https://example.com/mix1.mp3";  // Same URL
    std::string url3 = "https://example.com/mix2.mp3";  // Different URL

    auto id1 = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(url1);
    auto id2 = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(url2);
    auto id3 = AutoVibez::Utils::HashIdUtils::generateIdFromUrl(url3);

    // Same input should produce same output (deterministic hash)
    EXPECT_EQ(id1, id2);

    // Different input should produce different output
    EXPECT_NE(id1, id3);
    EXPECT_NE(id2, id3);
}
