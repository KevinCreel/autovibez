#include "overlay_messages.hpp"

#include <gtest/gtest.h>

using namespace AutoVibez::Utils;

class OverlayMessagesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset initialization state before each test
        // Note: We can't easily reset the static state, so tests should be order-independent
    }

    void TearDown() override {
        // Clean up after each test
    }
};

// Test registry initialization
TEST_F(OverlayMessagesTest, InitializationBehavior) {
    // Call initializeMessages explicitly
    OverlayMessages::initializeMessages();

    // Test that initialization is idempotent (safe to call multiple times)
    OverlayMessages::initializeMessages();
    OverlayMessages::initializeMessages();

    // No direct way to test initialization state, but we can test that messages work
    auto config = OverlayMessages::createMessage("mix_info", "Artist", "Title");
    EXPECT_EQ(config.formatter(), "Artist - Title");
}

// Test basic mix_info message creation
TEST_F(OverlayMessagesTest, MixInfoMessageCreation) {
    auto config = OverlayMessages::createMessage("mix_info", "Test Artist", "Test Title");

    EXPECT_EQ(config.formatter(), "Test Artist - Test Title");
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test mix_info message with empty strings
TEST_F(OverlayMessagesTest, MixInfoMessageWithEmptyStrings) {
    auto config = OverlayMessages::createMessage("mix_info", "", "");

    EXPECT_EQ(config.formatter(), " - ");
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test mix_info message with special characters
TEST_F(OverlayMessagesTest, MixInfoMessageWithSpecialCharacters) {
    auto config = OverlayMessages::createMessage("mix_info", "Artist & Co.", "Title (Remix)");

    EXPECT_EQ(config.formatter(), "Artist & Co. - Title (Remix)");
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test mix_info message with unicode characters
TEST_F(OverlayMessagesTest, MixInfoMessageWithUnicodeCharacters) {
    auto config = OverlayMessages::createMessage("mix_info", "Ártist", "Títle");

    EXPECT_EQ(config.formatter(), "Ártist - Títle");
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test mix_info message with very long strings
TEST_F(OverlayMessagesTest, MixInfoMessageWithLongStrings) {
    std::string longArtist(100, 'A');
    std::string longTitle(100, 'T');

    auto config = OverlayMessages::createMessage("mix_info", longArtist, longTitle);

    EXPECT_EQ(config.formatter(), longArtist + " - " + longTitle);
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test unknown message name
TEST_F(OverlayMessagesTest, UnknownMessageName) {
    auto config = OverlayMessages::createMessage("unknown_message", "arg1", "arg2");

    EXPECT_EQ(config.formatter(), "Unknown message");
    EXPECT_EQ(config.duration.count(), 3000);
    EXPECT_FALSE(config.colorTransition);
}

// Test empty message name
TEST_F(OverlayMessagesTest, EmptyMessageName) {
    auto config = OverlayMessages::createMessage("", "arg1", "arg2");

    EXPECT_EQ(config.formatter(), "Unknown message");
    EXPECT_EQ(config.duration.count(), 3000);
    EXPECT_FALSE(config.colorTransition);
}

// Test mix_info with insufficient arguments
TEST_F(OverlayMessagesTest, MixInfoWithInsufficientArguments) {
    auto config = OverlayMessages::createMessage("mix_info", "OnlyOneArg");

    // This should trigger the fallback case since we only provided 1 argument
    EXPECT_EQ(config.formatter(), "Unknown message");
    EXPECT_EQ(config.duration.count(), 3000);
    EXPECT_FALSE(config.colorTransition);
}

// Test mix_info with no arguments
TEST_F(OverlayMessagesTest, MixInfoWithNoArguments) {
    auto config = OverlayMessages::createMessage("mix_info");

    // This should trigger the fallback case
    EXPECT_EQ(config.formatter(), "Unknown message");
    EXPECT_EQ(config.duration.count(), 3000);
    EXPECT_FALSE(config.colorTransition);
}

// Test mix_info with too many arguments
TEST_F(OverlayMessagesTest, MixInfoWithTooManyArguments) {
    auto config = OverlayMessages::createMessage("mix_info", "Artist", "Title", "ExtraArg");

    // Should still work with first two arguments
    EXPECT_EQ(config.formatter(), "Artist - Title");
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test argument type conversion
TEST_F(OverlayMessagesTest, ArgumentTypeConversion) {
    // Test with std::string arguments (should use specialized template)
    std::string artist = "String Artist";
    std::string title = "String Title";

    auto config = OverlayMessages::createMessage("mix_info", artist, title);

    EXPECT_EQ(config.formatter(), "String Artist - String Title");
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test const char* arguments
TEST_F(OverlayMessagesTest, ConstCharPointerArguments) {
    const char* artist = "Const Artist";
    const char* title = "Const Title";

    auto config = OverlayMessages::createMessage("mix_info", artist, title);

    EXPECT_EQ(config.formatter(), "Const Artist - Const Title");
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test mixed argument types
TEST_F(OverlayMessagesTest, MixedArgumentTypes) {
    std::string artist = "Mixed Artist";
    const char* title = "Mixed Title";

    auto config = OverlayMessages::createMessage("mix_info", artist, title);

    EXPECT_EQ(config.formatter(), "Mixed Artist - Mixed Title");
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test message consistency across multiple calls
TEST_F(OverlayMessagesTest, MessageConsistency) {
    auto config1 = OverlayMessages::createMessage("mix_info", "Artist", "Title");
    auto config2 = OverlayMessages::createMessage("mix_info", "Artist", "Title");

    EXPECT_EQ(config1.formatter(), config2.formatter());
    EXPECT_EQ(config1.duration.count(), config2.duration.count());
    EXPECT_EQ(config1.colorTransition, config2.colorTransition);
}

// Test different message types (when more are added)
TEST_F(OverlayMessagesTest, FutureMessageTypeSupport) {
    // Test that the system handles unknown message types gracefully
    auto config = OverlayMessages::createMessage("future_message", "param1", "param2", "param3");

    EXPECT_EQ(config.formatter(), "Unknown message");
    EXPECT_EQ(config.duration.count(), 3000);
    EXPECT_FALSE(config.colorTransition);
}

// Test message registry robustness
TEST_F(OverlayMessagesTest, MessageRegistryRobustness) {
    // Test rapid creation of many messages
    for (int i = 0; i < 100; ++i) {
        auto config =
            OverlayMessages::createMessage("mix_info", "Artist" + std::to_string(i), "Title" + std::to_string(i));

        EXPECT_EQ(config.formatter(), "Artist" + std::to_string(i) + " - Title" + std::to_string(i));
        EXPECT_EQ(config.duration.count(), 20000);
        EXPECT_TRUE(config.colorTransition);
    }
}

// Test edge cases with whitespace
TEST_F(OverlayMessagesTest, WhitespaceHandling) {
    auto config = OverlayMessages::createMessage("mix_info", "  Artist  ", "  Title  ");

    EXPECT_EQ(config.formatter(), "  Artist   -   Title  ");
    EXPECT_EQ(config.duration.count(), 20000);
    EXPECT_TRUE(config.colorTransition);
}

// Test case sensitivity
TEST_F(OverlayMessagesTest, CaseSensitivity) {
    auto config1 = OverlayMessages::createMessage("mix_info", "Artist", "Title");
    auto config2 = OverlayMessages::createMessage("MIX_INFO", "Artist", "Title");

    // Message names should be case sensitive
    EXPECT_EQ(config1.formatter(), "Artist - Title");
    EXPECT_EQ(config2.formatter(), "Unknown message");
}

// Test lambda capture behavior
TEST_F(OverlayMessagesTest, LambdaCaptureCorrectness) {
    std::string artist = "Captured Artist";
    std::string title = "Captured Title";

    auto config = OverlayMessages::createMessage("mix_info", artist, title);

    // Modify original strings to ensure lambda captured values, not references
    artist = "Modified Artist";
    title = "Modified Title";

    // The formatter should still return the original captured values
    EXPECT_EQ(config.formatter(), "Captured Artist - Captured Title");
}
