#include "utils/path_utils.hpp"

#include <gtest/gtest.h>

TEST(PathUtilsTest, JoinPath) {
    // Test basic path joining
    EXPECT_EQ(AutoVibez::Utils::PathUtils::joinPath("/home/user", "mixes"), "/home/user/mixes");
    EXPECT_EQ(AutoVibez::Utils::PathUtils::joinPath("/home/user/", "mixes"), "/home/user/mixes");

    // Test with absolute paths (std::filesystem behavior)
    EXPECT_EQ(AutoVibez::Utils::PathUtils::joinPath("/home/user", "/mixes"), "/mixes");

    // Test with empty components
    EXPECT_EQ(AutoVibez::Utils::PathUtils::joinPath("", "mixes"), "mixes");
    EXPECT_EQ(AutoVibez::Utils::PathUtils::joinPath("/home/user", ""), "/home/user/");
}

TEST(PathUtilsTest, GetFileExtension) {
    // Test basic file extensions
    EXPECT_EQ(AutoVibez::Utils::PathUtils::getFileExtension("file.mp3"), "mp3");
    EXPECT_EQ(AutoVibez::Utils::PathUtils::getFileExtension("file.MP3"), "mp3");
    EXPECT_EQ(AutoVibez::Utils::PathUtils::getFileExtension("file.yaml"), "yaml");

    // Test with paths
    EXPECT_EQ(AutoVibez::Utils::PathUtils::getFileExtension("/path/to/file.mp3"), "mp3");
    EXPECT_EQ(AutoVibez::Utils::PathUtils::getFileExtension("file"), "");

    // Test edge cases
    EXPECT_EQ(AutoVibez::Utils::PathUtils::getFileExtension(""), "");
    EXPECT_EQ(AutoVibez::Utils::PathUtils::getFileExtension("file."), "");
}
