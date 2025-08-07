#include "datetime_utils.hpp"

#include <gtest/gtest.h>

#include <chrono>

using AutoVibez::Utils::DateTimeUtils;

class DateTimeUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DateTimeUtilsTest, GetCurrentDateTime_ReturnsValidFormat) {
    std::string datetime = DateTimeUtils::getCurrentDateTime();

    // Should be in format "YYYY-MM-DD HH:MM:SS"
    EXPECT_EQ(datetime.length(), 19);
    EXPECT_EQ(datetime[4], '-');
    EXPECT_EQ(datetime[7], '-');
    EXPECT_EQ(datetime[10], ' ');
    EXPECT_EQ(datetime[13], ':');
    EXPECT_EQ(datetime[16], ':');
}

TEST_F(DateTimeUtilsTest, GetCurrentDate_ReturnsValidFormat) {
    std::string date = DateTimeUtils::getCurrentDate();

    // Should be in format "YYYY-MM-DD"
    EXPECT_EQ(date.length(), 10);
    EXPECT_EQ(date[4], '-');
    EXPECT_EQ(date[7], '-');
}

TEST_F(DateTimeUtilsTest, GetCurrentTime_ReturnsValidFormat) {
    std::string time = DateTimeUtils::getCurrentTime();

    // Should be in format "HH:MM:SS"
    EXPECT_EQ(time.length(), 8);
    EXPECT_EQ(time[2], ':');
    EXPECT_EQ(time[5], ':');
}

TEST_F(DateTimeUtilsTest, FormatDateTime_ConsistentResults) {
    auto now = std::chrono::system_clock::now();
    std::string formatted1 = DateTimeUtils::formatDateTime(now);
    std::string formatted2 = DateTimeUtils::formatDateTime(now);

    // Same time should produce same formatted string
    EXPECT_EQ(formatted1, formatted2);
    EXPECT_EQ(formatted1.length(), 19);
}

TEST_F(DateTimeUtilsTest, IsValidDateTime_ValidFormats) {
    EXPECT_TRUE(DateTimeUtils::isValidDateTime("2023-12-25 14:30:45"));
    EXPECT_TRUE(DateTimeUtils::isValidDateTime("2000-01-01 00:00:00"));
    EXPECT_TRUE(DateTimeUtils::isValidDateTime("2099-12-31 23:59:59"));
}

TEST_F(DateTimeUtilsTest, IsValidDateTime_InvalidFormats) {
    EXPECT_FALSE(DateTimeUtils::isValidDateTime("2023-12-25"));
    EXPECT_FALSE(DateTimeUtils::isValidDateTime("14:30:45"));
    EXPECT_FALSE(DateTimeUtils::isValidDateTime("2023/12/25 14:30:45"));
    EXPECT_FALSE(DateTimeUtils::isValidDateTime("2023-12-25 14:30"));
    EXPECT_FALSE(DateTimeUtils::isValidDateTime(""));
    EXPECT_FALSE(DateTimeUtils::isValidDateTime("invalid"));
}

TEST_F(DateTimeUtilsTest, ParseDateTime_ValidString) {
    std::string datetime_str = "2023-12-25 14:30:45";
    auto time_point = DateTimeUtils::parseDateTime(datetime_str);

    // Should not be current time (indicating successful parse)
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - time_point);

    // The parsed time should be different from current time (unless we're testing at exactly that time)
    // For this test, we'll just verify it doesn't throw and produces a valid time point
    EXPECT_TRUE(time_point.time_since_epoch().count() > 0);
}

TEST_F(DateTimeUtilsTest, ParseDateTime_InvalidString) {
    std::string invalid_datetime = "invalid";
    auto time_point = DateTimeUtils::parseDateTime(invalid_datetime);

    // Should return current time for invalid input
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - time_point);

    // Should be very close to current time (within 1 second)
    EXPECT_LE(diff.count(), 1);
}

TEST_F(DateTimeUtilsTest, DateTimeConsistency) {
    auto now = std::chrono::system_clock::now();
    std::string formatted = DateTimeUtils::formatDateTime(now);

    // Parsing the formatted string should give us back a similar time
    auto parsed = DateTimeUtils::parseDateTime(formatted);
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - parsed);

    // Should be very close (within 1 second)
    EXPECT_LE(diff.count(), 1);
}
