#include "json_utils.hpp"

#include <gtest/gtest.h>

using namespace AutoVibez::Utils;

class JsonUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(JsonUtilsTest, VectorToJsonArrayEmpty) {
    std::vector<std::string> empty_tags;
    std::string result = JsonUtils::vectorToJsonArray(empty_tags);
    EXPECT_EQ(result, "[]");
}

TEST_F(JsonUtilsTest, VectorToJsonArraySingleItem) {
    std::vector<std::string> tags = {"electronic"};
    std::string result = JsonUtils::vectorToJsonArray(tags);
    EXPECT_EQ(result, "[\"electronic\"]");
}

TEST_F(JsonUtilsTest, VectorToJsonArrayMultipleItems) {
    std::vector<std::string> tags = {"electronic", "house", "progressive"};
    std::string result = JsonUtils::vectorToJsonArray(tags);
    EXPECT_EQ(result, "[\"electronic\",\"house\",\"progressive\"]");
}

TEST_F(JsonUtilsTest, VectorToJsonArrayWithSpecialCharacters) {
    std::vector<std::string> tags = {"test\"quote", "test\\backslash", "test\nnewline"};
    std::string result = JsonUtils::vectorToJsonArray(tags);
    EXPECT_EQ(result, "[\"test\\\"quote\",\"test\\\\backslash\",\"test\\nnewline\"]");
}

TEST_F(JsonUtilsTest, JsonArrayToVectorEmpty) {
    std::string json = "[]";
    std::vector<std::string> result = JsonUtils::jsonArrayToVector(json);
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonUtilsTest, JsonArrayToVectorEmptyString) {
    std::string json = "";
    std::vector<std::string> result = JsonUtils::jsonArrayToVector(json);
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonUtilsTest, JsonArrayToVectorSingleItem) {
    std::string json = "[\"electronic\"]";
    std::vector<std::string> result = JsonUtils::jsonArrayToVector(json);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "electronic");
}

TEST_F(JsonUtilsTest, JsonArrayToVectorMultipleItems) {
    std::string json = "[\"electronic\",\"house\",\"progressive\"]";
    std::vector<std::string> result = JsonUtils::jsonArrayToVector(json);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "electronic");
    EXPECT_EQ(result[1], "house");
    EXPECT_EQ(result[2], "progressive");
}

TEST_F(JsonUtilsTest, JsonArrayToVectorWithWhitespace) {
    std::string json = "[ \"electronic\" , \"house\" , \"progressive\" ]";
    std::vector<std::string> result = JsonUtils::jsonArrayToVector(json);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "electronic");
    EXPECT_EQ(result[1], "house");
    EXPECT_EQ(result[2], "progressive");
}

TEST_F(JsonUtilsTest, JsonArrayToVectorWithEscapedCharacters) {
    // Test basic escape sequences that our simple parser handles
    std::string json = "[\"test\\\"quote\",\"test_tag\"]";
    std::vector<std::string> result = JsonUtils::jsonArrayToVector(json);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "test\"quote");
    EXPECT_EQ(result[1], "test_tag");
}

TEST_F(JsonUtilsTest, RoundTripConversion) {
    std::vector<std::string> original = {"electronic", "house", "progressive", "test\"quote"};
    std::string json = JsonUtils::vectorToJsonArray(original);
    std::vector<std::string> result = JsonUtils::jsonArrayToVector(json);

    ASSERT_EQ(result.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(result[i], original[i]);
    }
}

TEST_F(JsonUtilsTest, JsonArrayToVectorMalformedJson) {
    std::string json = "[electronic, house, progressive]";  // Missing quotes
    std::vector<std::string> result = JsonUtils::jsonArrayToVector(json);
    // Should return empty vector for malformed JSON
    EXPECT_TRUE(result.empty());
}

TEST_F(JsonUtilsTest, JsonArrayToVectorWithEmptyStrings) {
    std::string json = "[\"\",\"electronic\",\"\"]";
    std::vector<std::string> result = JsonUtils::jsonArrayToVector(json);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "");
    EXPECT_EQ(result[1], "electronic");
    EXPECT_EQ(result[2], "");
}

TEST_F(JsonUtilsTest, VectorToJsonArrayWithEmptyStrings) {
    std::vector<std::string> tags = {"", "electronic", ""};
    std::string result = JsonUtils::vectorToJsonArray(tags);
    EXPECT_EQ(result, "[\"\",\"electronic\",\"\"]");
}
