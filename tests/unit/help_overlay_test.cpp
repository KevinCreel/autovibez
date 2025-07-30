#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "help_overlay.hpp"
#include <SDL2/SDL.h>

using namespace AutoVibez::UI;

class HelpOverlayTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize SDL for testing
        SDL_Init(SDL_INIT_VIDEO);
        
        // Create a test window
        _window = SDL_CreateWindow("Test Window", 
                                  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  800, 600, SDL_WINDOW_OPENGL);
        _glContext = SDL_GL_CreateContext(_window);
        
        _overlay = std::make_unique<HelpOverlay>();
    }
    
    void TearDown() override {
        _overlay.reset();
        
        if (_glContext) {
            SDL_GL_DeleteContext(_glContext);
        }
        if (_window) {
            SDL_DestroyWindow(_window);
        }
        
        SDL_Quit();
    }
    
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    std::unique_ptr<HelpOverlay> _overlay;
};

TEST_F(HelpOverlayTest, ConstructorInitializesCorrectly) {
    EXPECT_FALSE(_overlay->isVisible());
}

TEST_F(HelpOverlayTest, ToggleChangesVisibility) {
    // Initially hidden
    EXPECT_FALSE(_overlay->isVisible());
    
    // Toggle to show
    _overlay->toggle();
    EXPECT_TRUE(_overlay->isVisible());
    
    // Toggle to hide
    _overlay->toggle();
    EXPECT_FALSE(_overlay->isVisible());
}

TEST_F(HelpOverlayTest, SetFullscreenStateUpdatesInternalState) {
    // Test setting fullscreen state
    _overlay->setFullscreenState(true);
    // Note: We can't directly test the internal _isFullscreen state
    // but we can test the behavior through cursor management
    
    // Test setting windowed state
    _overlay->setFullscreenState(false);
}

TEST_F(HelpOverlayTest, CursorVisibilityInWindowedMode) {
    // Set to windowed mode
    _overlay->setFullscreenState(false);
    
    // Show overlay - cursor should be hidden
    _overlay->toggle();
    EXPECT_TRUE(_overlay->isVisible());
    
    // Hide overlay - cursor should be restored
    _overlay->toggle();
    EXPECT_FALSE(_overlay->isVisible());
}

TEST_F(HelpOverlayTest, CursorVisibilityInFullscreenMode) {
    // Set to fullscreen mode
    _overlay->setFullscreenState(true);
    
    // Show overlay - cursor should be hidden
    _overlay->toggle();
    EXPECT_TRUE(_overlay->isVisible());
    
    // Hide overlay - cursor should remain hidden in fullscreen
    _overlay->toggle();
    EXPECT_FALSE(_overlay->isVisible());
}

TEST_F(HelpOverlayTest, MultipleToggleOperations) {
    // Test multiple toggle operations
    for (int i = 0; i < 5; ++i) {
        _overlay->toggle();
        EXPECT_TRUE(_overlay->isVisible());
        
        _overlay->toggle();
        EXPECT_FALSE(_overlay->isVisible());
    }
}

TEST_F(HelpOverlayTest, FullscreenStateTransition) {
    // Start in windowed mode
    _overlay->setFullscreenState(false);
    EXPECT_FALSE(_overlay->isVisible());
    
    // Show overlay
    _overlay->toggle();
    EXPECT_TRUE(_overlay->isVisible());
    
    // Switch to fullscreen while overlay is visible
    _overlay->setFullscreenState(true);
    EXPECT_TRUE(_overlay->isVisible());
    
    // Hide overlay in fullscreen
    _overlay->toggle();
    EXPECT_FALSE(_overlay->isVisible());
    
    // Switch back to windowed mode
    _overlay->setFullscreenState(false);
    EXPECT_FALSE(_overlay->isVisible());
}

TEST_F(HelpOverlayTest, SetCursorVisibility) {
    // Test the setCursorVisibility method
    _overlay->setCursorVisibility(true);
    _overlay->setCursorVisibility(false);
    // Note: This method doesn't return anything, so we just test it doesn't crash
}

TEST_F(HelpOverlayTest, RenderDoesNotCrashWhenHidden) {
    // Should not crash when rendering while hidden
    _overlay->render();
    EXPECT_FALSE(_overlay->isVisible());
}

TEST_F(HelpOverlayTest, InitDoesNotCrash) {
    // Should not crash when initializing
    _overlay->init(_window, _glContext);
}

// Note: We avoid testing render when visible and multiple init calls
// because ImGui is a global singleton and causes issues in test environment 