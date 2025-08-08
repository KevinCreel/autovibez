#include <gtest/gtest.h>
#include "utils/constants.hpp"

TEST(ConstantsTest, AudioConstants) {
    // Test critical audio constants
    EXPECT_EQ(Constants::DEFAULT_SAMPLE_RATE, 44100);
    EXPECT_EQ(Constants::DEFAULT_CHANNELS, 2);
    EXPECT_EQ(Constants::MAX_VOLUME, 100);
    EXPECT_EQ(Constants::MIN_VOLUME, 0);
    EXPECT_EQ(Constants::BITS_PER_SAMPLE, 16);
}

TEST(ConstantsTest, FileValidationConstants) {
    // Test file validation constants
    EXPECT_EQ(Constants::MIN_MP3_FILE_SIZE, 1024);
    EXPECT_EQ(Constants::ID3V2_HEADER_SIZE, 10);
    EXPECT_EQ(Constants::MAX_FILENAME_LENGTH, 200);
}

TEST(ConstantsTest, StringConstants) {
    // Test critical string constants
    EXPECT_STREQ(StringConstants::MP3_EXTENSION, ".mp3");
    EXPECT_STREQ(StringConstants::UNKNOWN_ARTIST, "Unknown Artist");
    EXPECT_STREQ(StringConstants::DEFAULT_GENRE, "Electronic");
    EXPECT_STREQ(StringConstants::APP_TITLE, "AutoVibez");
}
