#pragma once

#include <memory>

#include "message_overlay.hpp"

namespace AutoVibez::UI {

/**
 * @brief Simple wrapper for MessageOverlay
 *
 * This class provides a simple interface for using the MessageOverlay
 * in the main AutoVibez application, with themed message methods.
 */
class MessageOverlayWrapper {
public:
    MessageOverlayWrapper();
    ~MessageOverlayWrapper();

    /**
     * @brief Initialize the message overlay integration
     * @param window SDL window pointer
     * @param glContext OpenGL context
     */
    void init(SDL_Window* window, SDL_GLContext glContext);

    /**
     * @brief Render the message overlay
     */
    void render();

    /**
     * @brief Show a message
     * @param content Message text
     * @param duration How long to display (default: 20 seconds)
     */
    void showMessage(const std::string& content, std::chrono::milliseconds duration = std::chrono::milliseconds(20000));

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
     * @brief Set the window size for positioning calculations
     * @param width Window width
     * @param height Window height
     */
    void setWindowSize(int width, int height);

    /**
     * @brief Get the underlying MessageOverlay
     * @return Pointer to the MessageOverlay
     */
    MessageOverlay* getMessageOverlay() const;

    /**
     * @brief Enable/disable color transition effect
     * @param enabled Whether to enable color transitions
     */
    void setColorTransition(bool enabled);

private:
    std::unique_ptr<MessageOverlay> _messageOverlay;
    bool _initialized = false;
};

}  // namespace AutoVibez::UI
