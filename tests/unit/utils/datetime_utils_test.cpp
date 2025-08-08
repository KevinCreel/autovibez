#include "utils/datetime_utils.hpp"

#include <gtest/gtest.h>

#include <regex>
#include <thread>
#include <vector>

TEST(DateTimeUtilsTest, GetCurrentDateTime) {
    // Test that current datetime is returned in correct format
    auto datetime = AutoVibez::Utils::DateTimeUtils::getCurrentDateTime();

    // Should not be empty
    EXPECT_FALSE(datetime.empty());

    // Should be in format "YYYY-MM-DD HH:MM:SS" (19 characters)
    EXPECT_EQ(datetime.length(), 19);

    // Should match the expected datetime pattern (more robust than testing exact positions)
    static const std::regex datetimePattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(datetime, datetimePattern));

    // Should contain expected separators at correct positions
    EXPECT_EQ(datetime[4], '-');   // Year-Month separator
    EXPECT_EQ(datetime[7], '-');   // Month-Day separator
    EXPECT_EQ(datetime[10], ' ');  // Date-Time separator
    EXPECT_EQ(datetime[13], ':');  // Hour-Minute separator
    EXPECT_EQ(datetime[16], ':');  // Minute-Second separator
}

TEST(DateTimeUtilsTest, GetCurrentDateTimeConsistency) {
    // Test that multiple calls within a short time return consistent format
    auto datetime1 = AutoVibez::Utils::DateTimeUtils::getCurrentDateTime();
    auto datetime2 = AutoVibez::Utils::DateTimeUtils::getCurrentDateTime();

    // Both should have same format
    static const std::regex datetimePattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(datetime1, datetimePattern));
    EXPECT_TRUE(std::regex_match(datetime2, datetimePattern));

    // Both should be 19 characters
    EXPECT_EQ(datetime1.length(), 19);
    EXPECT_EQ(datetime2.length(), 19);
}

TEST(DateTimeUtilsTest, IsValidDateTime) {
    // Test valid datetime formats
    EXPECT_TRUE(AutoVibez::Utils::DateTimeUtils::isValidDateTime("2024-01-15 14:30:25"));
    EXPECT_TRUE(AutoVibez::Utils::DateTimeUtils::isValidDateTime("2023-12-31 23:59:59"));
    EXPECT_TRUE(AutoVibez::Utils::DateTimeUtils::isValidDateTime("2024-02-29 00:00:00"));  // Leap year

    // Test invalid datetime formats (due to format mismatch)
    EXPECT_FALSE(AutoVibez::Utils::DateTimeUtils::isValidDateTime(""));
    EXPECT_FALSE(AutoVibez::Utils::DateTimeUtils::isValidDateTime("not-a-datetime"));
    EXPECT_FALSE(AutoVibez::Utils::DateTimeUtils::isValidDateTime("2024-1-15 14:30:25"));  // Missing leading zero
    EXPECT_FALSE(AutoVibez::Utils::DateTimeUtils::isValidDateTime("2024-01-15 4:30:25"));  // Missing leading zero
    EXPECT_FALSE(AutoVibez::Utils::DateTimeUtils::isValidDateTime("2024-01-15 14:3:25"));  // Missing leading zero

    // Test format-only validation (regex doesn't validate actual date values)
    EXPECT_TRUE(
        AutoVibez::Utils::DateTimeUtils::isValidDateTime("2024-13-01 14:30:25"));  // Invalid month but correct format
    EXPECT_TRUE(
        AutoVibez::Utils::DateTimeUtils::isValidDateTime("2024-01-32 14:30:25"));  // Invalid day but correct format
    EXPECT_TRUE(
        AutoVibez::Utils::DateTimeUtils::isValidDateTime("2024-01-15 25:30:25"));  // Invalid hour but correct format
}
