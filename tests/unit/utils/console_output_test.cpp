#include "console_output.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sstream>

using namespace AutoVibez::Utils;
using ::testing::HasSubstr;

class ConsoleOutputTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Capture cout for testing
        originalCout = std::cout.rdbuf();
        std::cout.rdbuf(testOutput.rdbuf());

        // Reset static state
        ConsoleOutput::enableColors(true);
        ConsoleOutput::enableEmojis(true);
        ConsoleOutput::setVerbose(true);
    }

    void TearDown() override {
        // Restore cout
        std::cout.rdbuf(originalCout);
    }

    std::string getOutput() {
        std::string output = testOutput.str();
        testOutput.str("");
        testOutput.clear();
        return output;
    }

    std::ostringstream testOutput;
    std::streambuf* originalCout;
};

TEST_F(ConsoleOutputTest, BasicPrintOutput) {
    ConsoleOutput::print("Hello World");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Hello World"));
}

TEST_F(ConsoleOutputTest, PrintlnAddsNewline) {
    ConsoleOutput::println("Test message");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Test message"));
    EXPECT_THAT(output, HasSubstr("\n"));
}

TEST_F(ConsoleOutputTest, ColorizeWrapsTextWithColors) {
    std::string colorized = ConsoleOutput::colorize("test", Colors::RED);
    EXPECT_THAT(colorized, HasSubstr(Colors::RED));
    EXPECT_THAT(colorized, HasSubstr("test"));
    EXPECT_THAT(colorized, HasSubstr(Colors::RESET));
}

TEST_F(ConsoleOutputTest, StylizeWrapsTextWithStyles) {
    std::string stylized = ConsoleOutput::stylize("test", Styles::BOLD);
    EXPECT_THAT(stylized, HasSubstr(Styles::BOLD));
    EXPECT_THAT(stylized, HasSubstr("test"));
    EXPECT_THAT(stylized, HasSubstr(Colors::RESET));
}

TEST_F(ConsoleOutputTest, WithEmojiAddsEmojiPrefix) {
    std::string withEmoji = ConsoleOutput::withEmoji(Symbols::SUCCESS, "Done");
    EXPECT_THAT(withEmoji, HasSubstr(Symbols::SUCCESS));
    EXPECT_THAT(withEmoji, HasSubstr("Done"));
}

TEST_F(ConsoleOutputTest, LogLevelOutputFormatsCorrectly) {
    ConsoleOutput::info("Info message");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Info message"));
}

TEST_F(ConsoleOutputTest, SuccessOutputIncludesSuccessSymbol) {
    ConsoleOutput::success("Success message");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Success message"));
    EXPECT_THAT(output, HasSubstr(Symbols::SUCCESS));
}

TEST_F(ConsoleOutputTest, ErrorOutputIncludesErrorSymbol) {
    ConsoleOutput::error("Error message");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Error message"));
    EXPECT_THAT(output, HasSubstr(Symbols::ERROR));
}

TEST_F(ConsoleOutputTest, WarningOutputIncludesWarningSymbol) {
    ConsoleOutput::warning("Warning message");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Warning message"));
    EXPECT_THAT(output, HasSubstr(Symbols::WARNING));
}

TEST_F(ConsoleOutputTest, DebugOutputOnlyShownWhenVerbose) {
    ConsoleOutput::setVerbose(false);
    ConsoleOutput::debug("Debug message");
    std::string output = getOutput();
    EXPECT_TRUE(output.empty());

    ConsoleOutput::setVerbose(true);
    ConsoleOutput::debug("Debug message");
    output = getOutput();
    EXPECT_THAT(output, HasSubstr("Debug message"));
}

TEST_F(ConsoleOutputTest, MusicEventFormatsCorrectly) {
    ConsoleOutput::musicEvent("Track changed", "New Song");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Track changed"));
    EXPECT_THAT(output, HasSubstr("New Song"));
    EXPECT_THAT(output, HasSubstr(Symbols::MUSIC));
}

TEST_F(ConsoleOutputTest, VolumeChangeShowsFinalVolume) {
    ConsoleOutput::volumeChange(50, 60);
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("60%"));
    EXPECT_THAT(output, HasSubstr(Symbols::VOLUME));
    // Should not show the old volume
    EXPECT_THAT(output, Not(HasSubstr("50%")));
}

TEST_F(ConsoleOutputTest, PresetChangeIncludesPresetName) {
    ConsoleOutput::presetChange("Cool Preset");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Cool Preset"));
    EXPECT_THAT(output, HasSubstr(Symbols::SPARKLES));
}

TEST_F(ConsoleOutputTest, MixInfoDisplaysAllComponents) {
    ConsoleOutput::mixInfo("Artist Name", "Song Title", "Electronic");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Artist Name"));
    EXPECT_THAT(output, HasSubstr("Song Title"));
    EXPECT_THAT(output, HasSubstr("Electronic"));
    EXPECT_THAT(output, HasSubstr(Symbols::MUSIC));
}

TEST_F(ConsoleOutputTest, KeyValueFormatsCorrectly) {
    ConsoleOutput::keyValue("Key", "Value");
    std::string output = getOutput();
    EXPECT_THAT(output, HasSubstr("Key:"));
    EXPECT_THAT(output, HasSubstr("Value"));
}

TEST_F(ConsoleOutputTest, ColorsCanBeDisabled) {
    ConsoleOutput::enableColors(false);
    std::string colorized = ConsoleOutput::colorize("test", Colors::RED);
    EXPECT_EQ(colorized, "test");
}

TEST_F(ConsoleOutputTest, EmojisCanBeDisabled) {
    ConsoleOutput::enableEmojis(false);
    std::string withEmoji = ConsoleOutput::withEmoji(Symbols::SUCCESS, "Done");
    EXPECT_EQ(withEmoji, "Done");
}

TEST_F(ConsoleOutputTest, ConsoleStreamOperatorChaining) {
    ConsoleStream stream;
    stream << "Hello" << " " << "World" << 42;
    // Test passes if no exception is thrown
    SUCCEED();
}

TEST_F(ConsoleOutputTest, ConsoleStreamFluentInterface) {
    ConsoleStream stream;
    stream.color(Colors::RED).style(Styles::BOLD).emoji(Symbols::FIRE);
    // Test passes if no exception is thrown
    SUCCEED();
}

TEST_F(ConsoleOutputTest, ConsoleFactoryFunctions) {
    Console::info("Test info message");
    Console::success("Test success message");
    Console::warning("Test warning message");
    Console::error("Test error message");
    Console::debug("Test debug message");
    Console::music("Test music message");
    // Test passes if no exception is thrown
    SUCCEED();
}
