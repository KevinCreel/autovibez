#include <gtest/gtest.h>
#include "utils/string_utils.hpp"

TEST(StringUtilsTest, TrimRemovesWhitespace) {
    // Test basic trimming
    EXPECT_EQ(AutoVibez::Utils::StringUtils::trim("  hello  "), "hello");
    EXPECT_EQ(AutoVibez::Utils::StringUtils::trim("hello"), "hello");
    EXPECT_EQ(AutoVibez::Utils::StringUtils::trim("  "), "");
}

TEST(StringUtilsTest, TrimHandlesEdgeCases) {
    // Test edge cases
    EXPECT_EQ(AutoVibez::Utils::StringUtils::trim(""), "");
    EXPECT_EQ(AutoVibez::Utils::StringUtils::trim("\t\n\r"), "");
    EXPECT_EQ(AutoVibez::Utils::StringUtils::trim("\thello\t"), "hello");
    EXPECT_EQ(AutoVibez::Utils::StringUtils::trim("hello\n"), "hello");
}

TEST(StringUtilsTest, ToLowerConvertsCase) {
    // Test case conversion
    EXPECT_EQ(AutoVibez::Utils::StringUtils::toLower("HELLO"), "hello");
    EXPECT_EQ(AutoVibez::Utils::StringUtils::toLower("Hello World"), "hello world");
    EXPECT_EQ(AutoVibez::Utils::StringUtils::toLower("hello"), "hello");
    EXPECT_EQ(AutoVibez::Utils::StringUtils::toLower(""), "");
}

TEST(StringUtilsTest, StartsWithChecksPrefix) {
    // Test prefix checking
    EXPECT_TRUE(AutoVibez::Utils::StringUtils::startsWith("hello world", "hello"));
    EXPECT_TRUE(AutoVibez::Utils::StringUtils::startsWith("hello", "hello"));
    EXPECT_FALSE(AutoVibez::Utils::StringUtils::startsWith("hello world", "world"));
    EXPECT_FALSE(AutoVibez::Utils::StringUtils::startsWith("", "hello"));
    EXPECT_TRUE(AutoVibez::Utils::StringUtils::startsWith("hello", ""));
}
