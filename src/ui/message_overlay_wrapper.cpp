#include "message_overlay_wrapper.hpp"

namespace AutoVibez {
namespace UI {

MessageOverlayWrapper::MessageOverlayWrapper() {}

MessageOverlayWrapper::~MessageOverlayWrapper() = default;

void MessageOverlayWrapper::init(SDL_Window* window, SDL_GLContext glContext) {
    if (_initialized) {
        return;  // Already initialized
    }

    _messageOverlay = std::make_unique<MessageOverlay>();
    if (_messageOverlay) {
        _messageOverlay->init(window, glContext);
    }

    _initialized = true;
}

void MessageOverlayWrapper::render() {
    if (_messageOverlay) {
        _messageOverlay->render();
    }
}

void MessageOverlayWrapper::showMessage(const std::string& content, std::chrono::milliseconds duration) {
    if (_messageOverlay) {
        _messageOverlay->showMessage(content, duration);
    }
}

void MessageOverlayWrapper::hideMessage() {
    if (_messageOverlay) {
        _messageOverlay->hideMessage();
    }
}

bool MessageOverlayWrapper::isVisible() const {
    return _messageOverlay ? _messageOverlay->isVisible() : false;
}

void MessageOverlayWrapper::setWindowSize(int width, int height) {
    if (_messageOverlay) {
        _messageOverlay->setWindowSize(width, height);
    }
}

}  // namespace UI
}  // namespace AutoVibez
