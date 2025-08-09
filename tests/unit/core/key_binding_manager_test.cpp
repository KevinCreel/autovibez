#include "key_binding_manager.hpp"

#include <gtest/gtest.h>

#include <cstring>

using namespace AutoVibez::Core;

class KeyBindingManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        keyBindingManager = std::make_unique<KeyBindingManager>();
        // Initialize with default bindings for consistent test state
        keyBindingManager->loadDefaultBindings();
    }

    void TearDown() override {
        keyBindingManager.reset();
    }

    // Helper to create SDL key events
    SDL_Event createKeyEvent(SDL_Keycode keycode, SDL_Keymod modifiers = KMOD_NONE) {
        SDL_Event event;
        memset(&event, 0, sizeof(event));  // Initialize all fields to zero
        event.type = SDL_KEYDOWN;
        event.key.type = SDL_KEYDOWN;
        event.key.timestamp = 0;
        event.key.windowID = 0;
        event.key.state = SDL_PRESSED;
        event.key.repeat = 0;
        event.key.keysym.scancode = SDL_SCANCODE_UNKNOWN;
        event.key.keysym.sym = keycode;
        event.key.keysym.mod = static_cast<Uint16>(modifiers);
        return event;
    }

    std::unique_ptr<KeyBindingManager> keyBindingManager;
};

// Test default bindings are loaded correctly
TEST_F(KeyBindingManagerTest, DefaultBindingsLoaded) {
    // Verify some key default bindings exist
    auto categories = keyBindingManager->getCategories();

    EXPECT_FALSE(categories.empty());
    EXPECT_TRUE(std::find(categories.begin(), categories.end(), "MIX MANAGEMENT") != categories.end());
    EXPECT_TRUE(std::find(categories.begin(), categories.end(), "VISUALIZER CONTROLS") != categories.end());
    EXPECT_TRUE(std::find(categories.begin(), categories.end(), "AUDIO CONTROLS") != categories.end());
}

// Test action registration and callback execution
TEST_F(KeyBindingManagerTest, ActionRegistrationAndExecution) {
    // Arrange
    bool callbackExecuted = false;
    auto callback = [&callbackExecuted]() { callbackExecuted = true; };

    // Act
    keyBindingManager->registerAction(KeyAction::SHOW_MIX_INFO, callback);
    auto event = createKeyEvent(SDLK_i);
    bool handled = keyBindingManager->handleKey(&event);

    // Assert
    EXPECT_TRUE(handled);
    EXPECT_TRUE(callbackExecuted);
}

// Test unregistered action callback is not called
TEST_F(KeyBindingManagerTest, UnregisteredActionNotCalled) {
    // Arrange
    bool callbackExecuted = false;
    auto callback = [&callbackExecuted]() { callbackExecuted = true; };

    // Don't register the callback for SHOW_MIX_INFO
    auto event = createKeyEvent(SDLK_i);

    // Act
    bool handled = keyBindingManager->handleKey(&event);

    // Assert - should still handle the key (binding exists) but callback not executed
    auto bindings = keyBindingManager->getBindingsByCategory("MIX MANAGEMENT");
    bool foundShowMixInfo = false;
    for (const auto& binding : bindings) {
        if (binding.action == KeyAction::SHOW_MIX_INFO) {
            foundShowMixInfo = true;
            break;
        }
    }
    ASSERT_TRUE(foundShowMixInfo);  // Ensure binding exists
    EXPECT_TRUE(handled);           // Key should be handled even without callback
    EXPECT_FALSE(callbackExecuted);
}

// Test unknown key is not handled
TEST_F(KeyBindingManagerTest, UnknownKeyNotHandled) {
    // Arrange - Use a key that's unlikely to be bound
    auto event = createKeyEvent(SDLK_BACKQUOTE);  // backtick key

    // Act
    bool handled = keyBindingManager->handleKey(&event);

    // Assert
    EXPECT_FALSE(handled);
}

// Test key rebinding functionality
TEST_F(KeyBindingManagerTest, KeyRebinding) {
    // Arrange
    bool callbackExecuted = false;
    auto callback = [&callbackExecuted]() { callbackExecuted = true; };
    keyBindingManager->registerAction(KeyAction::SHOW_MIX_INFO, callback);

    // Act - Rebind I key to different key
    keyBindingManager->rebindKey(KeyAction::SHOW_MIX_INFO, SDLK_p, KMOD_NONE);

    // Test old key no longer works
    auto oldEvent = createKeyEvent(SDLK_i);
    bool oldHandled = keyBindingManager->handleKey(&oldEvent);

    // Test new key works
    auto newEvent = createKeyEvent(SDLK_p);
    bool newHandled = keyBindingManager->handleKey(&newEvent);

    // Assert
    EXPECT_FALSE(oldHandled);
    EXPECT_TRUE(newHandled);
    EXPECT_TRUE(callbackExecuted);
}

// Test clearing bindings
TEST_F(KeyBindingManagerTest, ClearBinding) {
    // Arrange
    bool callbackExecuted = false;
    auto callback = [&callbackExecuted]() { callbackExecuted = true; };
    keyBindingManager->registerAction(KeyAction::SHOW_MIX_INFO, callback);

    // Act
    keyBindingManager->clearBinding(KeyAction::SHOW_MIX_INFO);
    auto event = createKeyEvent(SDLK_i);
    bool handled = keyBindingManager->handleKey(&event);

    // Assert
    EXPECT_FALSE(handled);
    EXPECT_FALSE(callbackExecuted);
}

// Test custom binding registration
TEST_F(KeyBindingManagerTest, CustomBindingRegistration) {
    // Arrange
    KeyBinding customBinding(SDLK_x, KMOD_SHIFT, KeyAction::TOGGLE_MUTE, "Custom mute toggle", "CUSTOM");
    bool callbackExecuted = false;
    auto callback = [&callbackExecuted]() { callbackExecuted = true; };

    // Act
    keyBindingManager->registerBinding(customBinding);
    keyBindingManager->registerAction(KeyAction::TOGGLE_MUTE, callback);

    // Test without modifier first (should not work)
    auto eventNoMod = createKeyEvent(SDLK_x, KMOD_NONE);
    bool handledNoMod = keyBindingManager->handleKey(&eventNoMod);

    // Test with modifier (should work)
    auto event = createKeyEvent(SDLK_x, KMOD_SHIFT);
    bool handled = keyBindingManager->handleKey(&event);

    // Assert
    EXPECT_FALSE(handledNoMod);  // Should not handle X without Shift
    EXPECT_TRUE(handled);
    EXPECT_TRUE(callbackExecuted);
}

// Test key display string generation
TEST_F(KeyBindingManagerTest, KeyDisplayStringGeneration) {
    // Act & Assert
    EXPECT_EQ(keyBindingManager->getKeyDisplayString(SDLK_a, KMOD_NONE), "A");
    EXPECT_EQ(keyBindingManager->getKeyDisplayString(SDLK_a, KMOD_CTRL), "Ctrl+A");
    EXPECT_EQ(keyBindingManager->getKeyDisplayString(SDLK_a, KMOD_SHIFT), "Shift+A");
    EXPECT_EQ(keyBindingManager->getKeyDisplayString(SDLK_a, static_cast<SDL_Keymod>(KMOD_CTRL | KMOD_SHIFT)),
              "Ctrl+Shift+A");
}

// Test getting bindings by category
TEST_F(KeyBindingManagerTest, GetBindingsByCategory) {
    // Act
    auto mixBindings = keyBindingManager->getBindingsByCategory("MIX MANAGEMENT");
    auto audioBindings = keyBindingManager->getBindingsByCategory("AUDIO CONTROLS");
    auto emptyBindings = keyBindingManager->getBindingsByCategory("NONEXISTENT");

    // Assert
    EXPECT_FALSE(mixBindings.empty());
    EXPECT_FALSE(audioBindings.empty());
    EXPECT_TRUE(emptyBindings.empty());

    // Verify bindings are in correct category
    for (const auto& binding : mixBindings) {
        EXPECT_EQ(binding.category, "MIX MANAGEMENT");
    }
}

// Test binding description retrieval
TEST_F(KeyBindingManagerTest, BindingDescriptionRetrieval) {
    // Act
    std::string description = keyBindingManager->getBindingDescription(KeyAction::SHOW_MIX_INFO);

    // Assert
    EXPECT_FALSE(description.empty());
    EXPECT_EQ(description, "Show current mix info");
}

// Test context management
TEST_F(KeyBindingManagerTest, ContextManagement) {
    // Arrange
    std::string testContext = "TEST_CONTEXT";

    // Act
    keyBindingManager->setContext(testContext);
    std::string currentContext = keyBindingManager->getCurrentContext();

    // Assert
    EXPECT_EQ(currentContext, testContext);
}

// Test multiple callbacks for same action
TEST_F(KeyBindingManagerTest, MultipleCallbacksOverwrite) {
    // Arrange
    bool firstCallbackExecuted = false;
    bool secondCallbackExecuted = false;
    auto firstCallback = [&firstCallbackExecuted]() { firstCallbackExecuted = true; };
    auto secondCallback = [&secondCallbackExecuted]() { secondCallbackExecuted = true; };

    // Act
    keyBindingManager->registerAction(KeyAction::SHOW_MIX_INFO, firstCallback);
    keyBindingManager->registerAction(KeyAction::SHOW_MIX_INFO, secondCallback);  // Should overwrite

    auto event = createKeyEvent(SDLK_i);
    keyBindingManager->handleKey(&event);

    // Assert
    EXPECT_FALSE(firstCallbackExecuted);
    EXPECT_TRUE(secondCallbackExecuted);
}

// Test KeyBinding equality operators
TEST_F(KeyBindingManagerTest, KeyBindingEquality) {
    // Arrange
    KeyBinding binding1(SDLK_a, KMOD_CTRL, KeyAction::TOGGLE_MUTE, "Test", "TEST");
    KeyBinding binding2(SDLK_a, KMOD_CTRL, KeyAction::TOGGLE_MUTE, "Test", "TEST");
    KeyBinding binding3(SDLK_b, KMOD_CTRL, KeyAction::TOGGLE_MUTE, "Test", "TEST");

    // Assert
    EXPECT_TRUE(binding1 == binding2);
    EXPECT_FALSE(binding1 == binding3);
    EXPECT_FALSE(binding1 != binding2);
    EXPECT_TRUE(binding1 != binding3);
}

// Test edge case with null event
TEST_F(KeyBindingManagerTest, NullEventHandling) {
    // Act & Assert
    EXPECT_FALSE(keyBindingManager->handleKey(nullptr));
}

// Test edge case with non-keydown event
TEST_F(KeyBindingManagerTest, NonKeydownEventHandling) {
    // Arrange
    SDL_Event event;
    event.type = SDL_KEYUP;  // Not a keydown event
    event.key.keysym.sym = SDLK_i;

    // Act
    bool handled = keyBindingManager->handleKey(&event);

    // Assert
    EXPECT_FALSE(handled);
}

// Test getting all categories
TEST_F(KeyBindingManagerTest, GetAllCategories) {
    // Act
    auto categories = keyBindingManager->getCategories();

    // Assert
    EXPECT_FALSE(categories.empty());
    // Should contain expected categories
    EXPECT_TRUE(std::find(categories.begin(), categories.end(), "MIX MANAGEMENT") != categories.end());
    EXPECT_TRUE(std::find(categories.begin(), categories.end(), "VISUALIZER CONTROLS") != categories.end());
    EXPECT_TRUE(std::find(categories.begin(), categories.end(), "AUDIO CONTROLS") != categories.end());
    EXPECT_TRUE(std::find(categories.begin(), categories.end(), "APPLICATION") != categories.end());
}

// Test bindings with display strings
TEST_F(KeyBindingManagerTest, BindingsWithDisplayStrings) {
    // Act
    auto bindingsMap = keyBindingManager->getBindingsWithDisplayStrings("MIX MANAGEMENT");

    // Assert
    EXPECT_FALSE(bindingsMap.empty());

    // Check that we have display strings for actions
    for (const auto& [action, displayString] : bindingsMap) {
        EXPECT_FALSE(displayString.empty());
    }
}

// Test callback exception handling (behavior test - exceptions currently propagate)
TEST_F(KeyBindingManagerTest, CallbackExceptionHandling) {
    // Arrange
    auto throwingCallback = []() { throw std::runtime_error("Test exception"); };
    keyBindingManager->registerAction(KeyAction::SHOW_MIX_INFO, throwingCallback);

    // Act & Assert - Current implementation allows exceptions to propagate
    auto event = createKeyEvent(SDLK_i);
    // Note: This test verifies the current behavior where exceptions propagate
    EXPECT_THROW(keyBindingManager->handleKey(&event), std::runtime_error);
}
