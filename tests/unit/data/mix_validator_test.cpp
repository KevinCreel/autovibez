#include "mix_validator.hpp"

#include <gtest/gtest.h>

#include "mix_metadata.hpp"

using namespace AutoVibez::Data;

class MixValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        validator = std::make_unique<MixValidator>();

        // Create a valid mix for testing
        validMix.id = "test-id-123";
        validMix.title = "Test Mix";
        validMix.artist = "Test Artist";
        validMix.genre = "Electronic";
        validMix.url = "https://example.com/mix.mp3";
        validMix.duration_seconds = 3600;
    }

    void TearDown() override {}

    std::unique_ptr<MixValidator> validator;
    Mix validMix;
};

TEST_F(MixValidatorTest, ValidateValidMix) {
    auto result = validator->validate(validMix);
    EXPECT_TRUE(result.isValid);
    EXPECT_TRUE(result.errorMessage.empty());
}

TEST_F(MixValidatorTest, ValidateEmptyId) {
    validMix.id = "";
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_FALSE(result.errorMessage.empty());
    EXPECT_NE(result.errorMessage.find("ID cannot be empty"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateWhitespaceOnlyId) {
    validMix.id = "   ";
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("ID cannot be just whitespace"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateVeryLongId) {
    validMix.id = std::string(101, 'x');  // 101 characters
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("ID is too long"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateEmptyTitle) {
    validMix.title = "";
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("title cannot be empty"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateWhitespaceOnlyTitle) {
    validMix.title = "   \t\n  ";
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("title cannot be just whitespace"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateTitleSameAsId) {
    validMix.title = validMix.id;
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("title cannot be the same as ID"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateVeryLongTitle) {
    validMix.title = std::string(501, 'x');  // 501 characters
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("title is too long"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateEmptyArtist) {
    validMix.artist = "";
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("artist cannot be empty"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateUnknownArtist) {
    validMix.artist = "Unknown Artist";
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("artist cannot be 'Unknown Artist'"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateWhitespaceOnlyArtist) {
    validMix.artist = "   ";
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("artist cannot be just whitespace"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateVeryLongArtist) {
    validMix.artist = std::string(201, 'x');  // 201 characters
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("artist is too long"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateEmptyGenre) {
    validMix.genre = "";
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("genre cannot be empty"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateWhitespaceOnlyGenre) {
    validMix.genre = "   ";
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("genre cannot be just whitespace"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateVeryLongGenre) {
    validMix.genre = std::string(101, 'x');  // 101 characters
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("genre is too long"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateZeroDuration) {
    validMix.duration_seconds = 0;
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("duration must be greater than 0"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateNegativeDuration) {
    validMix.duration_seconds = -100;
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("duration must be greater than 0"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateVeryLongDuration) {
    validMix.duration_seconds = 86401;  // More than 24 hours
    auto result = validator->validate(validMix);
    EXPECT_FALSE(result.isValid);
    EXPECT_NE(result.errorMessage.find("duration is too long"), std::string::npos);
}

TEST_F(MixValidatorTest, ValidateIdDirectly) {
    auto result = validator->validateId("valid-id-123");
    EXPECT_TRUE(result.isValid);

    result = validator->validateId("");
    EXPECT_FALSE(result.isValid);
}

TEST_F(MixValidatorTest, ValidateTitleDirectly) {
    auto result = validator->validateTitle("Valid Title", "different-id");
    EXPECT_TRUE(result.isValid);

    result = validator->validateTitle("same-id", "same-id");
    EXPECT_FALSE(result.isValid);
}

TEST_F(MixValidatorTest, ValidateArtistDirectly) {
    auto result = validator->validateArtist("Valid Artist");
    EXPECT_TRUE(result.isValid);

    result = validator->validateArtist("Unknown Artist");
    EXPECT_FALSE(result.isValid);
}

TEST_F(MixValidatorTest, ValidateGenreDirectly) {
    auto result = validator->validateGenre("Electronic");
    EXPECT_TRUE(result.isValid);

    result = validator->validateGenre("");
    EXPECT_FALSE(result.isValid);
}

TEST_F(MixValidatorTest, ValidateDurationDirectly) {
    auto result = validator->validateDuration(3600);
    EXPECT_TRUE(result.isValid);

    result = validator->validateDuration(0);
    EXPECT_FALSE(result.isValid);

    result = validator->validateDuration(-100);
    EXPECT_FALSE(result.isValid);
}

TEST_F(MixValidatorTest, ValidationResultBooleanConversion) {
    ValidationResult valid(true);
    ValidationResult invalid(false, "Error message");

    EXPECT_TRUE(valid);
    EXPECT_FALSE(invalid);

    if (valid) {
        SUCCEED();
    } else {
        FAIL() << "Valid result should evaluate to true";
    }

    if (!invalid) {
        SUCCEED();
    } else {
        FAIL() << "Invalid result should evaluate to false";
    }
}

// Behavioral Scenario Tests

TEST_F(MixValidatorTest, RealisticMixFromMusicService) {
    // Test a realistic mix that might come from a music service
    Mix realisticMix;
    realisticMix.id = "soundcloud-mix-123456";
    realisticMix.title = "Deep House Sessions Vol. 1";
    realisticMix.artist = "DJ Producer";
    realisticMix.genre = "Deep House";
    realisticMix.url = "https://soundcloud.com/djproducer/deep-house-sessions-vol-1";
    realisticMix.duration_seconds = 4200;  // 70 minutes

    auto result = validator->validate(realisticMix);
    EXPECT_TRUE(result) << "Realistic mix should be valid: " << result.errorMessage;
}

TEST_F(MixValidatorTest, UserInputWithTypicalErrors) {
    // Test what happens when user enters data with common mistakes
    Mix userInputMix;
    userInputMix.id = "  my-mix-id  ";  // User might add extra spaces
    userInputMix.title = "My Awesome Mix";
    userInputMix.artist = "  My Artist Name  ";  // Extra spaces
    userInputMix.genre = "Electronic";
    userInputMix.duration_seconds = 3600;

    // Note: This assumes the validator trims whitespace (if it doesn't, it should fail)
    auto result = validator->validate(userInputMix);
    // The behavior depends on whether validator trims - test the actual behavior
    if (result.isValid) {
        SUCCEED() << "Validator handles whitespace trimming";
    } else {
        EXPECT_NE(result.errorMessage.find("whitespace"), std::string::npos)
            << "If validation fails on whitespace, error should mention it";
    }
}

TEST_F(MixValidatorTest, ImportedMixWithMinimalRequiredFields) {
    // Test a mix with only the absolute minimum required fields
    Mix minimalMix;
    minimalMix.id = "min-1";
    minimalMix.title = "T";           // Single character title
    minimalMix.artist = "A";          // Single character artist
    minimalMix.genre = "G";           // Single character genre
    minimalMix.duration_seconds = 1;  // Minimum valid duration

    auto result = validator->validate(minimalMix);
    EXPECT_TRUE(result) << "Minimal valid mix should pass: " << result.errorMessage;
}

TEST_F(MixValidatorTest, BatchValidationScenario) {
    // Test what happens when validating multiple mixes in sequence
    std::vector<Mix> mixBatch;

    // Create a batch of mixes with various validity states
    for (int i = 0; i < 5; ++i) {
        Mix mix;
        mix.id = "batch-mix-" + std::to_string(i);
        mix.title = (i == 2) ? "" : "Batch Mix " + std::to_string(i);  // Mix 2 has empty title
        mix.artist = (i == 3) ? "Unknown Artist" : "Batch Artist";     // Mix 3 has forbidden artist
        mix.genre = "Electronic";
        mix.duration_seconds = (i == 4) ? -100 : 3600;  // Mix 4 has negative duration
        mixBatch.push_back(mix);
    }

    std::vector<bool> validationResults;
    for (const auto& mix : mixBatch) {
        auto result = validator->validate(mix);
        validationResults.push_back(result.isValid);
    }

    // Should have valid results for mixes 0, 1 and invalid for 2, 3, 4
    EXPECT_TRUE(validationResults[0]);   // Valid
    EXPECT_TRUE(validationResults[1]);   // Valid
    EXPECT_FALSE(validationResults[2]);  // Empty title
    EXPECT_FALSE(validationResults[3]);  // Unknown Artist
    EXPECT_FALSE(validationResults[4]);  // Negative duration
}

TEST_F(MixValidatorTest, EdgeCaseDurationValues) {
    // Test boundary conditions for duration
    struct DurationTestCase {
        int duration;
        bool shouldBeValid;
        std::string description;
    };

    std::vector<DurationTestCase> testCases = {{1, true, "minimum valid duration"},
                                               {30, true, "30 seconds"},
                                               {3600, true, "1 hour"},
                                               {86400, true, "24 hours exactly"},
                                               {86401, false, "just over 24 hours"},
                                               {0, false, "zero duration"},
                                               {-1, false, "negative duration"}};

    for (const auto& testCase : testCases) {
        Mix testMix = validMix;
        testMix.duration_seconds = testCase.duration;

        auto result = validator->validate(testMix);
        EXPECT_EQ(result.isValid, testCase.shouldBeValid)
            << "Duration " << testCase.duration << " (" << testCase.description << ") " << "should be "
            << (testCase.shouldBeValid ? "valid" : "invalid") << ". Error: " << result.errorMessage;
    }
}
