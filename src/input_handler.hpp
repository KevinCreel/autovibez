#pragma once

#include <SDL2/SDL.h>
#include <functional>

class AutoVibezApp; // Forward declaration

class InputHandler {
public:
    explicit InputHandler(AutoVibezApp* app);
    
    /**
     * Handle key press events
     */
    void handleKeyPress(SDL_Event* event);
    
    /**
     * Handle mouse events
     */
    void handleMouseEvent(SDL_Event* event);
    
    /**
     * Handle window events
     */
    void handleWindowEvent(SDL_Event* event);
    
    /**
     * Process all pending events
     */
    void processEvents();
    
private:
    AutoVibezApp* _app;
    
    /**
     * Handle keyboard input
     */
    void handleKeyboardInput(SDL_Keycode key, SDL_Keymod mod);
    
    /**
     * Handle mouse click
     */
    void handleMouseClick(int x, int y, int button);
    
    /**
     * Handle mouse drag
     */
    void handleMouseDrag(int x, int y);
    
    /**
     * Handle mouse wheel
     */
    void handleMouseWheel(int y);
    
    /**
     * Handle window resize
     */
    void handleWindowResize(int width, int height);
    
    /**
     * Handle quit event
     */
    void handleQuit();
    
    // Keyboard command handlers
    void handleQuitCommand(SDL_Keymod mod);
    void handleAudioInputCommand(SDL_Keymod mod);
    void handleMonitorCommands(SDL_Keymod mod, SDL_Keycode key);
    void handleBeatSensitivityControls(SDL_Keycode key);
    void handlePresetNavigation(SDL_Keycode key);
    void handleMixControls(SDL_Keymod mod);
    void handleGenreControls(SDL_Keymod mod);
    void handleHelpToggle();
    void handleFullscreenToggle();
    void handleAudioDeviceCycle();
}; 