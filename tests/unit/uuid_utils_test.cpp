#include "uuid_utils.hpp"

#include <gtest/gtest.h>

using AutoVibez::Utils::UuidUtils;

class UuidUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(UuidUtilsTest, GenerateIdFromUrl_ConsistentResults) {
    std::string url1 = "https://example.com/mix.mp3";
    std::string url2 = "https://example.com/mix.mp3";

    // Same URL should produce same ID
    std::string id1 = UuidUtils::generateIdFromUrl(url1);
    std::string id2 = UuidUtils::generateIdFromUrl(url2);

    EXPECT_EQ(id1, id2);
    EXPECT_FALSE(id1.empty());
    EXPECT_FALSE(id2.empty());
}

TEST_F(UuidUtilsTest, GenerateIdFromUrl_DifferentUrls) {
    std::string url1 = "https://example.com/mix1.mp3";
    std::string url2 = "https://example.com/mix2.mp3";

    // Different URLs should produce different IDs
    std::string id1 = UuidUtils::generateIdFromUrl(url1);
    std::string id2 = UuidUtils::generateIdFromUrl(url2);

    EXPECT_NE(id1, id2);
    EXPECT_FALSE(id1.empty());
    EXPECT_FALSE(id2.empty());
}

TEST_F(UuidUtilsTest, GenerateIdFromUrl_EmptyUrl) {
    std::string empty_url = "";
    std::string id = UuidUtils::generateIdFromUrl(empty_url);

    EXPECT_FALSE(id.empty());
}

TEST_F(UuidUtilsTest, UuidFormat) {
    std::string url = "https://example.com/test.mp3";
    std::string id = UuidUtils::generateIdFromUrl(url);

    // UUID should be in format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    EXPECT_EQ(id.length(), 36);
    EXPECT_EQ(id[8], '-');
    EXPECT_EQ(id[13], '-');
    EXPECT_EQ(id[18], '-');
    EXPECT_EQ(id[23], '-');

    // All characters should be hexadecimal
    for (char c : id) {
        if (c != '-') {
            EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
        }
    }
}
