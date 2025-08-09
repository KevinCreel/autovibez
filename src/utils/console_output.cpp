#include "console_output.hpp"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace AutoVibez::Utils {

// Static member initialization
bool ConsoleOutput::_colorsEnabled = true;
bool ConsoleOutput::_emojisEnabled = true;
bool ConsoleOutput::_verbose = false;

void ConsoleOutput::enableColors(bool enable) {
    _colorsEnabled = enable && isatty(fileno(stdout));
}

void ConsoleOutput::enableEmojis(bool enable) {
    _emojisEnabled = enable;
}

void ConsoleOutput::setVerbose(bool verbose) {
    _verbose = verbose;
}

void ConsoleOutput::print(const std::string& message, const std::string& color) {
    if (_colorsEnabled) {
        std::cout << color << message << Colors::RESET;
    } else {
        std::cout << message;
    }
}

void ConsoleOutput::println(const std::string& message, const std::string& color) {
    print(message, color);
    std::cout << std::endl;
}

void ConsoleOutput::printBold(const std::string& message, const std::string& color) {
    if (_colorsEnabled) {
        std::cout << Styles::BOLD << color << message << Colors::RESET << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

void ConsoleOutput::printItalic(const std::string& message, const std::string& color) {
    if (_colorsEnabled) {
        std::cout << Styles::ITALIC << color << message << Colors::RESET << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

void ConsoleOutput::printUnderline(const std::string& message, const std::string& color) {
    if (_colorsEnabled) {
        std::cout << Styles::UNDERLINE << color << message << Colors::RESET << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

void ConsoleOutput::debug(const std::string& message) {
    if (_verbose) {
        println(formatMessage(LogLevel::DEBUG, message));
    }
}

void ConsoleOutput::info(const std::string& message) {
    println(formatMessage(LogLevel::INFO, message));
}

void ConsoleOutput::success(const std::string& message) {
    println(formatMessage(LogLevel::SUCCESS, message));
}

void ConsoleOutput::warning(const std::string& message) {
    println(formatMessage(LogLevel::WARNING, message));
}

void ConsoleOutput::error(const std::string& message) {
    println(formatMessage(LogLevel::ERROR, message));
}

void ConsoleOutput::musicEvent(const std::string& event, const std::string& details) {
    std::string message = withEmoji(Symbols::MUSIC, event);
    if (!details.empty()) {
        message += " " + colorize(details, Colors::BRIGHT_CYAN);
    }
    println(message, Colors::MAGENTA);
}

void ConsoleOutput::volumeChange(int oldVolume, int newVolume) {
    std::string message = std::string(Symbols::VOLUME) + " Volume: " + std::to_string(newVolume) + "%";
    std::string color = (newVolume > oldVolume) ? Colors::BRIGHT_GREEN : Colors::BRIGHT_YELLOW;
    println(message, color);
}

void ConsoleOutput::presetChange(const std::string& presetName) {
    std::string message = std::string(Symbols::SPARKLES) + " Preset: " + colorize(presetName, Colors::BRIGHT_MAGENTA);
    println(message, Colors::CYAN);
}

void ConsoleOutput::mixInfo(const std::string& artist, const std::string& title, const std::string& genre) {
    std::ostringstream oss;
    oss << std::string(Symbols::MUSIC) << " Now Playing: ";
    oss << colorize(artist, Colors::BRIGHT_YELLOW) << " - ";
    oss << colorize(title, Colors::BRIGHT_WHITE) << " ";
    oss << colorize("[" + genre + "]", Colors::BRIGHT_CYAN);
    println(oss.str(), Colors::GREEN);
}

void ConsoleOutput::downloadProgress(const std::string& filename, int percentage) {
    std::string message = std::string(Symbols::DOWNLOAD) + " Downloading " + colorize(filename, Colors::BRIGHT_WHITE);
    println(message + " " + std::to_string(percentage) + "%", Colors::BLUE);
    progressBar(percentage, 30, "");
}

void ConsoleOutput::printBanner(const std::string& title) {
    // Calculate the visual width of content: title + emoji spacing
    // Each emoji displays as approximately 2 characters wide in most terminals
    std::string musicIcon = "🎵";
    size_t titleLength = title.length();
    size_t emojiVisualWidth = 2; // Each emoji takes about 2 character spaces visually
    size_t contentVisualWidth = 1 + emojiVisualWidth + 1 + titleLength + 1 + emojiVisualWidth + 1; // " 🎵 title 🎵 "
    
    // Add padding for a nice border
    size_t totalWidth = contentVisualWidth + 4; // 2 spaces on each side
    
    // Build the border strings
    std::string horizontalBorder = "";
    for (size_t i = 0; i < totalWidth; i++) {
        horizontalBorder += "═";
    }
    
    std::string topBorder = "╔" + horizontalBorder + "╗";
    std::string bottomBorder = "╚" + horizontalBorder + "╝";
    
    // Empty line
    std::string spaces = "";
    for (size_t i = 0; i < totalWidth; i++) {
        spaces += " ";
    }
    std::string emptyLine = "║" + spaces + "║";
    
    // Content line - build it with proper centering
    std::string content = " " + musicIcon + " " + title + " " + musicIcon + " ";
    size_t leftPadding = (totalWidth - contentVisualWidth) / 2;
    size_t rightPadding = totalWidth - contentVisualWidth - leftPadding;
    
    std::string leftSpaces = "";
    for (size_t i = 0; i < leftPadding; i++) {
        leftSpaces += " ";
    }
    
    std::string rightSpaces = "";
    for (size_t i = 0; i < rightPadding; i++) {
        rightSpaces += " ";
    }
    
    std::string titleLine = "║" + leftSpaces + content + rightSpaces + "║";
    
    println("", Colors::RESET);
    println(topBorder, Colors::BRIGHT_MAGENTA);
    println(emptyLine, Colors::BRIGHT_MAGENTA);
    println(titleLine, Colors::BRIGHT_CYAN);
    println(emptyLine, Colors::BRIGHT_MAGENTA);
    println(bottomBorder, Colors::BRIGHT_MAGENTA);
    println("", Colors::RESET);
}

void ConsoleOutput::printSection(const std::string& section) {
    std::string message = std::string(Symbols::GEAR) + " " + stylize(section, Styles::BOLD);
    println(message, Colors::BRIGHT_YELLOW);
    printSeparator('-', static_cast<int>(section.length()) + 4);
}

void ConsoleOutput::printSeparator(char character, int length) {
    println(std::string(length, character), Colors::BRIGHT_BLACK);
}

void ConsoleOutput::progressBar(int percentage, int width, const std::string& label) {
    int progress = static_cast<int>((percentage / 100.0) * width);

    std::ostringstream bar;
    if (!label.empty()) {
        bar << label << " ";
    }

    bar << "[";
    for (int i = 0; i < width; ++i) {
        if (i < progress) {
            bar << "█";
        } else {
            bar << "░";
        }
    }
    bar << "] " << percentage << "%";

    std::string color = (percentage >= 100)  ? Colors::BRIGHT_GREEN
                        : (percentage >= 50) ? Colors::BRIGHT_YELLOW
                                             : Colors::BRIGHT_RED;

    println(bar.str(), color);
}

void ConsoleOutput::spinner(const std::string& message) {
    static const char* frames[] = {"|", "/", "-", "\\", "|", "/", "-", "\\"};
    static int frameIndex = 0;

    std::cout << "\r" << frames[frameIndex] << " " << message << std::flush;
    frameIndex = (frameIndex + 1) % 8;
}

void ConsoleOutput::keyValue(const std::string& key, const std::string& value, const std::string& keyColor) {
    std::ostringstream oss;
    oss << colorize(key + ":", keyColor) << " " << colorize(value, Colors::BRIGHT_WHITE);
    println(oss.str());
}

void ConsoleOutput::listItem(const std::string& item, int level) {
    std::string indent(level * 2, ' ');
    std::string bullet = withEmoji(Symbols::BULLET, "");
    println(indent + bullet + " " + item, Colors::WHITE);
}

void ConsoleOutput::rainbow(const std::string& message) {
    if (!_colorsEnabled) {
        println(message);
        return;
    }

    const std::vector<std::string> rainbowColors = {Colors::RED,  Colors::YELLOW, Colors::GREEN,
                                                    Colors::CYAN, Colors::BLUE,   Colors::MAGENTA};

    for (size_t i = 0; i < message.length(); ++i) {
        std::string color = rainbowColors[i % rainbowColors.size()];
        print(std::string(1, message[i]), color);
    }
    std::cout << std::endl;
}

void ConsoleOutput::gradient(const std::string& message, const std::string& startColor, const std::string& endColor) {
    if (!_colorsEnabled) {
        println(message, startColor);
        return;
    }

    // Simple gradient effect by alternating colors
    for (size_t i = 0; i < message.length(); ++i) {
        std::string color = (i % 2 == 0) ? startColor : endColor;
        print(std::string(1, message[i]), color);
    }
    std::cout << std::endl;
}

std::string ConsoleOutput::colorize(const std::string& text, const std::string& color) {
    if (_colorsEnabled) {
        return color + text + Colors::RESET;
    }
    return text;
}

std::string ConsoleOutput::stylize(const std::string& text, const std::string& style) {
    if (_colorsEnabled) {
        return style + text + Colors::RESET;
    }
    return text;
}

std::string ConsoleOutput::withEmoji(const std::string& emoji, const std::string& text) {
    if (_emojisEnabled) {
        // Normalize emoji spacing for consistent alignment
        std::string normalizedEmoji = normalizeEmojiSpacing(emoji);
        return normalizedEmoji + " " + text;
    }
    return text;
}

std::string ConsoleOutput::formatMessage(LogLevel level, const std::string& message) {
    std::ostringstream oss;

    if (_emojisEnabled) {
        std::string emoji = getLevelEmoji(level);
        std::string normalizedEmoji = normalizeEmojiSpacing(emoji);
        oss << normalizedEmoji << " ";
    }

    std::string coloredMessage = colorize(message, getLevelColor(level));
    oss << coloredMessage;

    return oss.str();
}

std::string ConsoleOutput::getLevelColor(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return Colors::BRIGHT_BLACK;
        case LogLevel::INFO:
            return Colors::BRIGHT_BLUE;
        case LogLevel::SUCCESS:
            return Colors::BRIGHT_GREEN;
        case LogLevel::WARNING:
            return Colors::BRIGHT_YELLOW;
        case LogLevel::ERROR:
            return Colors::BRIGHT_RED;
        default:
            return Colors::WHITE;
    }
}

std::string ConsoleOutput::getLevelEmoji(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return Symbols::DEBUG;
        case LogLevel::INFO:
            return Symbols::INFO;
        case LogLevel::SUCCESS:
            return Symbols::SUCCESS;
        case LogLevel::WARNING:
            return Symbols::WARNING;
        case LogLevel::ERROR:
            return Symbols::ERROR;
        default:
            return "";
    }
}

std::string ConsoleOutput::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

int ConsoleOutput::calculateEmojiDisplayWidth(const std::string& emoji) {
    // Simple heuristic to calculate display width:
    // Most single emojis = 2 display columns
    // Emojis with variation selectors (️) may appear wider due to font rendering
    // This is a basic approach that works for most common cases

    // Count Unicode codepoints and variation selectors
    int width = 0;
    bool hasVariationSelector = emoji.find("\uFE0F") != std::string::npos;  // Variation Selector-16

    // Basic emoji detection - if it contains typical emoji ranges
    bool hasEmoji = false;
    for (size_t i = 0; i < emoji.length();) {
        unsigned char c = static_cast<unsigned char>(emoji[i]);
        if (c >= 0xF0) {  // 4-byte UTF-8 sequence (typical for emojis)
            hasEmoji = true;
            i += 4;
        } else if (c >= 0xE0) {  // 3-byte UTF-8 sequence
            i += 3;
        } else if (c >= 0xC0) {  // 2-byte UTF-8 sequence
            i += 2;
        } else {
            i += 1;
        }
    }

    if (hasEmoji) {
        // Most emojis take 2 display columns
        width = 2;
        // Emojis with variation selectors might appear to take different space
        if (hasVariationSelector) {
            width = 2;  // Keep consistent regardless of variation selector
        }
    } else {
        // Regular text characters
        width = static_cast<int>(emoji.length());
    }

    return width;
}

std::string ConsoleOutput::normalizeEmojiSpacing(const std::string& emoji) {
    // The core issue: emojis with variation selectors (️) appear to have inconsistent spacing
    // in many terminals. We need to normalize this visually.

    // Check if this emoji has a variation selector (EF B8 8F in UTF-8)
    bool hasVariationSelector = emoji.find("\uFE0F") != std::string::npos;

    if (hasVariationSelector) {
        // Emojis with variation selectors often appear to have less trailing space
        // Add an extra space to compensate
        return emoji + " ";
    }

    // Regular emojis get standard spacing
    return emoji;
}

// ConsoleStream implementation
ConsoleStream::ConsoleStream(const std::string& initialColor) : _currentColor(initialColor), _needsReset(false) {
    if (ConsoleOutput::isColorsEnabled()) {
        _buffer << initialColor;
        _needsReset = true;
    }
}

ConsoleStream::~ConsoleStream() {
    flush();
}

ConsoleStream& ConsoleStream::operator<<(const std::string& text) {
    _buffer << text;
    return *this;
}

ConsoleStream& ConsoleStream::operator<<(const char* text) {
    _buffer << text;
    return *this;
}

ConsoleStream& ConsoleStream::operator<<(int value) {
    _buffer << value;
    return *this;
}

ConsoleStream& ConsoleStream::operator<<(double value) {
    _buffer << value;
    return *this;
}

ConsoleStream& ConsoleStream::operator<<(bool value) {
    _buffer << (value ? "true" : "false");
    return *this;
}

ConsoleStream& ConsoleStream::color(const std::string& color) {
    if (ConsoleOutput::isColorsEnabled()) {
        _buffer << color;
        _currentColor = color;
        _needsReset = true;
    }
    return *this;
}

ConsoleStream& ConsoleStream::style(const std::string& style) {
    if (ConsoleOutput::isColorsEnabled()) {
        _buffer << style;
        _needsReset = true;
    }
    return *this;
}

ConsoleStream& ConsoleStream::emoji(const std::string& emoji) {
    if (ConsoleOutput::isEmojisEnabled()) {
        _buffer << emoji << " ";
    }
    return *this;
}

ConsoleStream& ConsoleStream::newline() {
    _buffer << "\n";
    return *this;
}

ConsoleStream& ConsoleStream::tab() {
    _buffer << "\t";
    return *this;
}

ConsoleStream& ConsoleStream::space() {
    _buffer << " ";
    return *this;
}

void ConsoleStream::flush() {
    if (_needsReset && ConsoleOutput::isColorsEnabled()) {
        _buffer << Colors::RESET;
    }
    std::cout << _buffer.str() << std::endl;
    _buffer.str("");
    _buffer.clear();
    _needsReset = false;
}

// Factory functions
namespace Console {
void info(const std::string& message) {
    ConsoleOutput::info(message);
}

void success(const std::string& message) {
    ConsoleOutput::success(message);
}

void warning(const std::string& message) {
    ConsoleOutput::warning(message);
}

void error(const std::string& message) {
    ConsoleOutput::error(message);
}

void debug(const std::string& message) {
    ConsoleOutput::debug(message);
}

void music(const std::string& message) {
    ConsoleOutput::musicEvent(message);
}
}  // namespace Console

}  // namespace AutoVibez::Utils
