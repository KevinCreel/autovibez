#include "input_handler.hpp"
#include "autovibez_app.hpp"
#include "constants.hpp"
#include "string_utils.hpp"
#include "console_output.hpp"
#include <iostream>

using AutoVibez::Core::AutoVibezApp;

InputHandler::InputHandler(AutoVibezApp* app)
    : _app(app) {
}

void InputHandler::processEvents() {
    SDL_Event evt;
    while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
            case SDL_WINDOWEVENT:
                handleWindowEvent(&evt);
                break;
            case SDL_MOUSEWHEEL:
                handleMouseEvent(&evt);
                break;
            case SDL_KEYDOWN:
                handleKeyPress(&evt);
                break;
            case SDL_KEYUP:
                // Handle volume key release
                if (evt.key.keysym.sym == SDLK_UP || evt.key.keysym.sym == SDLK_DOWN) {
                    // Volume key release handled in AutoVibezApp
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                handleMouseEvent(&evt);
                break;
            case SDL_QUIT:
                handleQuit();
                break;
        }
    }
}

void InputHandler::handleKeyPress(SDL_Event* event) {
    SDL_Keymod mod = (SDL_Keymod)event->key.keysym.mod;
    SDL_Keycode key = event->key.keysym.sym;
    
    handleKeyboardInput(key, mod);
}

void InputHandler::handleMouseEvent(SDL_Event* event) {
    switch (event->type) {
        case SDL_MOUSEWHEEL:
            handleMouseWheel(event->wheel.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                handleMouseClick(x, y, SDL_BUTTON_LEFT);
            } else if (event->button.button == SDL_BUTTON_RIGHT) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                handleMouseClick(x, y, SDL_BUTTON_RIGHT);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            // Handle mouse up if needed
            break;
    }
}

void InputHandler::handleWindowEvent(SDL_Event* event) {
    switch (event->window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            int w, h;
            SDL_GL_GetDrawableSize(_app->getWindow(), &w, &h);
            handleWindowResize(w, h);
            break;
    }
}

void InputHandler::handleKeyboardInput(SDL_Keycode key, SDL_Keymod mod) {
    // Check for modifier keys first
    if (mod & KMOD_LGUI || mod & KMOD_RGUI || mod & KMOD_LCTRL) {
        switch (key) {
            case SDLK_q:
                handleQuitCommand(mod);
                return;
            case SDLK_i:
                handleAudioInputCommand(mod);
                return;
            case SDLK_s:
            case SDLK_m:
            case SDLK_f:
                handleMonitorCommands(mod, key);
                return;
        }
    }
    
    // Handle regular keys
    switch (key) {
        case SDLK_b:
        case SDLK_j:
            handleBeatSensitivityControls(key);
            break;
        case SDLK_LEFTBRACKET:
        case SDLK_RIGHTBRACKET:
            handlePresetNavigation(key);
            break;
        case SDLK_g:
            handleGenreControls(mod);
            break;
        case SDLK_h:
            handleHelpToggle();
            break;
        case SDLK_F11:
            handleFullscreenToggle();
            break;
        case SDLK_TAB:
            handleAudioDeviceCycle();
            break;
        case SDLK_SPACE:
            handleMixControls(mod);
            break;
        case SDLK_UP:
        case SDLK_DOWN:
            // Volume controls handled in AutoVibezApp
            break;
    }
}

void InputHandler::handleMouseClick(int x, int y, int button) {
    if (button == SDL_BUTTON_LEFT) {
        // Scale coordinates for ProjectM
        float mousexscale = x / (float)_app->getWidth();
        float mouseyscale = (_app->getHeight() - y) / (float)_app->getHeight();
        _app->handleTouch(mousexscale, mouseyscale, 0);
    } else if (button == SDL_BUTTON_RIGHT) {
        // Handle right click
        float mousexscale = x / (float)_app->getWidth();
        float mouseyscale = (_app->getHeight() - y) / (float)_app->getHeight();
        _app->destroyTouch(mousexscale, mouseyscale);
    }
}

void InputHandler::handleMouseDrag(int x, int y) {
    float mousexscale = x / (float)_app->getWidth();
    float mouseyscale = (_app->getHeight() - y) / (float)_app->getHeight();
    _app->handleTouchDrag(mousexscale, mouseyscale, 0);
}

void InputHandler::handleMouseWheel(int y) {
    if (y > 0) {
        // Wheel up - previous preset
        _app->getPresetManager()->previousPreset();
    } else if (y < 0) {
        // Wheel down - next preset
        _app->getPresetManager()->nextPreset();
    }
}

void InputHandler::handleWindowResize(int width, int height) {
    _app->resizeWindow(width, height);
}

void InputHandler::handleQuit() {
    _app->done = true;
}

void InputHandler::handleQuitCommand(SDL_Keymod mod) {
    _app->done = true;
}

void InputHandler::handleAudioInputCommand(SDL_Keymod mod) {
    _app->toggleAudioInput();
}

void InputHandler::handleMonitorCommands(SDL_Keymod mod, SDL_Keycode key) {
    switch (key) {
        case SDLK_s:
            // Stretch monitors
            if (!_app->stretch) {
                _app->stretchMonitors();
                _app->stretch = true;
            } else {
                _app->toggleFullScreen();
                _app->stretch = false;
            }
            break;
        case SDLK_m:
            // Change monitor
            _app->nextMonitor();
            _app->stretch = false;
            break;
        case SDLK_f:
            // Fullscreen
            _app->toggleFullScreen();
            _app->stretch = false;
            break;
    }
}

void InputHandler::handleBeatSensitivityControls(SDL_Keycode key) {
    float currentSensitivity = _app->getBeatSensitivity();
    float newSensitivity;
    
    if (key == SDLK_b) {
        // Increase sensitivity
        newSensitivity = currentSensitivity + Constants::BEAT_SENSITIVITY_STEP;
        if (newSensitivity > Constants::MAX_BEAT_SENSITIVITY) {
            newSensitivity = Constants::MAX_BEAT_SENSITIVITY;
        }
    } else if (key == SDLK_j) {
        // Decrease sensitivity
        newSensitivity = currentSensitivity - Constants::BEAT_SENSITIVITY_STEP;
        if (newSensitivity < Constants::MIN_BEAT_SENSITIVITY) {
            newSensitivity = Constants::MIN_BEAT_SENSITIVITY;
        }
    } else {
        return;
    }
    
    _app->setBeatSensitivity(newSensitivity);
            ConsoleOutput::output("🎵 Beat Sensitivity: %.1f", newSensitivity);
}

void InputHandler::handlePresetNavigation(SDL_Keycode key) {
    if (key == SDLK_LEFTBRACKET) {
        _app->getPresetManager()->previousPreset();
    } else if (key == SDLK_RIGHTBRACKET) {
        _app->getPresetManager()->nextPreset();
    }
}

void InputHandler::handleMixControls(SDL_Keymod mod) {
    // Handle space key for random mix
    if (_app->isMixManagerInitialized()) {
        // Random mix logic handled in AutoVibezApp
    }
}

void InputHandler::handleGenreControls(SDL_Keymod mod) {
    if (!_app->isMixManagerInitialized()) return;
    
    if (mod & KMOD_LSHIFT || mod & KMOD_RSHIFT) {
        // Shift+G: Switch to random genre
        std::string newGenre = _app->getMixManager()->getRandomGenre();
        ConsoleOutput::output("🎼 Switched to genre: %s", newGenre.c_str());
    } else if (mod & KMOD_LCTRL || mod & KMOD_RCTRL) {
        // Ctrl+G: Show available genres
        auto genres = _app->getMixManager()->getAvailableGenres();
        ConsoleOutput::output("🎼 Available genres (%zu):", genres.size());
        for (const auto& genre : genres) {
            ConsoleOutput::output("  • %s", StringUtils::toTitleCase(genre).c_str());
        }
    } else {
        // G: Random mix in current genre
        // Genre mix logic handled in AutoVibezApp
    }
}

void InputHandler::handleHelpToggle() {
    _app->toggleHelp();
}

void InputHandler::handleFullscreenToggle() {
    _app->toggleFullScreen();
}

void InputHandler::handleAudioDeviceCycle() {
    _app->cycleAudioDevice();
} 