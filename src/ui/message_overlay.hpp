#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <imgui.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "imgui_manager.hpp"

namespace AutoVibez {
namespace UI {

/**
 * @brief Message overlay for displaying temporary messages with smooth transitions
 *
 * Provides a flexible system for displaying messages over the application window
 * with configurable timing, content, and smooth fade in/out transitions.
 */
class MessageOverlay {
public:
    /**
     * @brief Message configuration structure
     */
    struct MessageConfig {
        std::string content;                    ///< Message text content
        std::chrono::milliseconds duration;     ///< How long to display the message
        std::chrono::milliseconds fadeInTime;   ///< Fade in duration
        std::chrono::milliseconds fadeOutTime;  ///< Fade out duration
        ImVec4 backgroundColor;                 ///< Background color
        ImVec4 textColor;                       ///< Text color
        ImVec4 borderColor;                     ///< Border color
        float cornerRadius;                     ///< Corner radius for rounded corners
        float padding;                          ///< Padding around text
        bool showBorder;                        ///< Whether to show border
        bool centerText;                        ///< Whether to center text
        int maxWidth;                           ///< Maximum width of message box

        // Sexy visual effects (implementing one by one)
        bool useSlideAnimation = true;  ///< Slide in from bottom
        float slideDistance = 50.0f;    ///< Slide distance in pixels
    };

    MessageOverlay();
    ~MessageOverlay();

    /**
     * @brief Initialize the message overlay
     * @param window SDL window pointer
     * @param glContext OpenGL context
     */
    void init(SDL_Window* window, SDL_GLContext glContext);

    /**
     * @brief Render the message overlay
     */
    void render();

    /**
     * @brief Show a message with default configuration
     * @param content Message text
     * @param duration How long to display (default: 3 seconds)
     */
    void showMessage(const std::string& content, std::chrono::milliseconds duration = std::chrono::milliseconds(3000));

    /**
     * @brief Show a message with custom configuration
     * @param config Message configuration
     */
    void showMessage(const MessageConfig& config);

    /**
     * @brief Hide the current message immediately
     */
    void hideMessage();

    /**
     * @brief Check if a message is currently visible
     * @return true if message is visible
     */
    bool isVisible() const;

    /**
     * @brief Check if ImGui is ready for rendering
     * @return true if ImGui is initialized
     */
    bool isImGuiReady() const;

    /**
     * @brief Set the window size for positioning calculations
     * @param width Window width
     * @param height Window height
     */
    void setWindowSize(int width, int height);

    /**
     * @brief Get default message configuration
     * @return Default configuration
     */
    static MessageConfig getDefaultConfig();

    /**
     * @brief Get success message configuration
     * @return Success configuration (green theme)
     */
    static MessageConfig getSuccessConfig();

    /**
     * @brief Get error message configuration
     * @return Error configuration (red theme)
     */
    static MessageConfig getErrorConfig();

    /**
     * @brief Get warning message configuration
     * @return Warning configuration (yellow theme)
     */
    static MessageConfig getWarningConfig();

    /**
     * @brief Get info message configuration
     * @return Info configuration (blue theme)
     */
    static MessageConfig getInfoConfig();

private:
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    bool _initialized = false;
    bool _imguiReady = false;
    bool _visible = false;

    // Message state
    MessageConfig _currentConfig;
    std::chrono::steady_clock::time_point _startTime;
    std::chrono::steady_clock::time_point _fadeInEndTime;
    std::chrono::steady_clock::time_point _fadeOutStartTime;
    std::chrono::steady_clock::time_point _endTime;

    // Window dimensions for positioning
    int _windowWidth = 800;
    int _windowHeight = 600;

    // Animation state
    float _currentAlpha = 0.0f;
    float _targetAlpha = 1.0f;

    void initializeImGui();
    void updateAnimation();
    float calculateCurrentAlpha();
    void renderMessageBox();
    ImVec2 calculateMessagePosition();
};

}  // namespace UI
}  // namespace AutoVibez
