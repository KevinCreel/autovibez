#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace AutoVibez::Utils {

/**
 * @brief ANSI color codes for console output
 */
namespace Colors {
// Reset
constexpr const char* RESET = "\033[0m";

// Regular colors
constexpr const char* BLACK = "\033[30m";
constexpr const char* RED = "\033[31m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* BLUE = "\033[34m";
constexpr const char* MAGENTA = "\033[35m";
constexpr const char* CYAN = "\033[36m";
constexpr const char* WHITE = "\033[37m";

// Bright colors
constexpr const char* BRIGHT_BLACK = "\033[90m";
constexpr const char* BRIGHT_RED = "\033[91m";
constexpr const char* BRIGHT_GREEN = "\033[92m";
constexpr const char* BRIGHT_YELLOW = "\033[93m";
constexpr const char* BRIGHT_BLUE = "\033[94m";
constexpr const char* BRIGHT_MAGENTA = "\033[95m";
constexpr const char* BRIGHT_CYAN = "\033[96m";
constexpr const char* BRIGHT_WHITE = "\033[97m";

// Background colors
constexpr const char* BG_BLACK = "\033[40m";
constexpr const char* BG_RED = "\033[41m";
constexpr const char* BG_GREEN = "\033[42m";
constexpr const char* BG_YELLOW = "\033[43m";
constexpr const char* BG_BLUE = "\033[44m";
constexpr const char* BG_MAGENTA = "\033[45m";
constexpr const char* BG_CYAN = "\033[46m";
constexpr const char* BG_WHITE = "\033[47m";
}  // namespace Colors

/**
 * @brief Text styling codes
 */
namespace Styles {
constexpr const char* BOLD = "\033[1m";
constexpr const char* DIM = "\033[2m";
constexpr const char* ITALIC = "\033[3m";
constexpr const char* UNDERLINE = "\033[4m";
constexpr const char* BLINK = "\033[5m";
constexpr const char* REVERSE = "\033[7m";
constexpr const char* STRIKETHROUGH = "\033[9m";
}  // namespace Styles

/**
 * @brief Emoji and special characters for enhanced output
 */
namespace Symbols {
constexpr const char* SUCCESS = "✅";
constexpr const char* ERROR = "❌";
constexpr const char* WARNING = "⚠️";
constexpr const char* INFO = "ℹ️";
constexpr const char* DEBUG = "🐛";
constexpr const char* MUSIC = "🎵";
constexpr const char* VOLUME = "🔊";
constexpr const char* MUTE = "🔇";
constexpr const char* PLAY = "▶️";
constexpr const char* PAUSE = "⏸️";
constexpr const char* STOP = "⏹️";
constexpr const char* NEXT = "⏭️";
constexpr const char* PREV = "⏮️";
constexpr const char* STAR = "⭐";
constexpr const char* HEART = "❤️";
constexpr const char* FIRE = "🔥";
constexpr const char* ROCKET = "🚀";
constexpr const char* LIGHTNING = "⚡";
constexpr const char* WAVE = "🌊";
constexpr const char* SPARKLES = "✨";
constexpr const char* GEAR = "⚙️";
constexpr const char* DOWNLOAD = "⬇️";
constexpr const char* UPLOAD = "⬆️";
constexpr const char* FOLDER = "📁";
constexpr const char* FILE = "📄";
constexpr const char* LINK = "🔗";
constexpr const char* LOCK = "🔒";
constexpr const char* UNLOCK = "🔓";
constexpr const char* BULLET = "•";
constexpr const char* ARROW_RIGHT = "→";
constexpr const char* ARROW_LEFT = "←";
constexpr const char* ARROW_UP = "↑";
constexpr const char* ARROW_DOWN = "↓";
}  // namespace Symbols

/**
 * @brief Message levels for structured output
 */
enum class LogLevel { DEBUG, INFO, SUCCESS, WARNING, ERROR };

/**
 * @brief Sexy console output utility with colors, styles, and emojis
 */
class ConsoleOutput {
public:
    ConsoleOutput() = default;
    ~ConsoleOutput() = default;

    // Configuration
    static void enableColors(bool enable);
    static void enableEmojis(bool enable);
    static void setVerbose(bool verbose);

    // Basic colored output
    static void print(const std::string& message, const std::string& color = Colors::WHITE);
    static void println(const std::string& message, const std::string& color = Colors::WHITE);

    // Styled output
    static void printBold(const std::string& message, const std::string& color = Colors::WHITE);
    static void printItalic(const std::string& message, const std::string& color = Colors::WHITE);
    static void printUnderline(const std::string& message, const std::string& color = Colors::WHITE);

    // Level-based logging
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void success(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);

    // Music-specific outputs
    static void musicEvent(const std::string& event, const std::string& details = "");
    static void presetChange(const std::string& presetName);
    static void volumeChange(int oldVolume, int newVolume);
    static void mixInfo(const std::string& artist, const std::string& title, const std::string& genre);
    static void downloadProgress(const std::string& filename, int percentage);

    // Banners and headers
    static void printBanner(const std::string& title);
    static void printSection(const std::string& section);
    static void printSeparator(char character = '-', int length = 60);

    // Progress indicators
    static void progressBar(int percentage, int width = 40, const std::string& label = "");
    static void spinner(const std::string& message = "Loading...");

    // Table-like output
    static void keyValue(const std::string& key, const std::string& value, const std::string& keyColor = Colors::CYAN);
    static void listItem(const std::string& item, int level = 0);

    // Special effects
    static void rainbow(const std::string& message);
    static void gradient(const std::string& message, const std::string& startColor, const std::string& endColor);

    // Utilities
    static std::string colorize(const std::string& text, const std::string& color);
    static std::string stylize(const std::string& text, const std::string& style);
    static std::string withEmoji(const std::string& emoji, const std::string& text);

    // Public access to settings for ConsoleStream
    static bool isColorsEnabled() {
        return _colorsEnabled;
    }
    static bool isEmojisEnabled() {
        return _emojisEnabled;
    }

private:
    static bool _colorsEnabled;
    static bool _emojisEnabled;
    static bool _verbose;

    static std::string formatMessage(LogLevel level, const std::string& message);
    static std::string getLevelColor(LogLevel level);
    static std::string getLevelEmoji(LogLevel level);
    static std::string getCurrentTimestamp();
    static int calculateEmojiDisplayWidth(const std::string& emoji);
    static std::string normalizeEmojiSpacing(const std::string& emoji);
};

/**
 * @brief Stream-like interface for fluent console output
 */
class ConsoleStream {
public:
    ConsoleStream(const std::string& initialColor = Colors::WHITE);
    ~ConsoleStream();

    ConsoleStream& operator<<(const std::string& text);
    ConsoleStream& operator<<(const char* text);
    ConsoleStream& operator<<(int value);
    ConsoleStream& operator<<(double value);
    ConsoleStream& operator<<(bool value);

    ConsoleStream& color(const std::string& color);
    ConsoleStream& style(const std::string& style);
    ConsoleStream& emoji(const std::string& emoji);
    ConsoleStream& newline();
    ConsoleStream& tab();
    ConsoleStream& space();

    void flush();

private:
    std::ostringstream _buffer;
    std::string _currentColor;
    bool _needsReset;
};

/**
 * @brief Factory functions for common console outputs
 */
namespace Console {
void info(const std::string& message);
void success(const std::string& message);
void warning(const std::string& message);
void error(const std::string& message);
void debug(const std::string& message);
void music(const std::string& message);
}  // namespace Console

}  // namespace AutoVibez::Utils
