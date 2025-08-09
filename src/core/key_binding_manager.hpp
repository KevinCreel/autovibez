#pragma once

#include <SDL2/SDL.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace AutoVibez::Core {

/**
 * @brief Enumeration of all possible key actions in the application
 */
enum class KeyAction {
    // Mix Management
    NEXT_MIX,
    PREVIOUS_MIX,
    TOGGLE_FAVORITE,
    SHOW_MIX_INFO,

    // Visualizer Controls
    NEXT_PRESET,
    PREVIOUS_PRESET,

    // UI Controls
    TOGGLE_HELP_OVERLAY,
    TOGGLE_FULLSCREEN,
    TOGGLE_FAVORITES_FILTER,

    // Application
    QUIT_APPLICATION,

    // Genre Controls (discovered in build artifacts)
    NEXT_GENRE,
    PREVIOUS_GENRE,

    // Audio Controls
    TOGGLE_MUTE,
    VOLUME_UP,
    VOLUME_DOWN,
    TOGGLE_AUDIO_INPUT,
    CYCLE_AUDIO_DEVICE,

    // Beat/Visualization Controls
    INCREASE_BEAT_SENSITIVITY,
    DECREASE_BEAT_SENSITIVITY,
    PREVIOUS_PRESET_BRACKET,
    NEXT_PRESET_BRACKET,

    // Mix Playback Controls
    PAUSE_RESUME_MIX,
    RANDOM_PRESET,
    SOFT_DELETE_MIX,
    RANDOM_MIX_CURRENT_GENRE,
    RANDOM_GENRE_AND_MIX,

    // Display Controls
    STRETCH_MONITORS,
    CHANGE_MONITOR,

    // Help Overlay Controls
    TOGGLE_MIX_TABLE_FILTER,

    // Application Controls (with modifiers)
    QUIT_WITH_MODIFIER,
    FULLSCREEN_WITH_MODIFIER,

    // Add new actions here
    UNKNOWN  // Must be last
};

/**
 * @brief Represents a single key binding
 */
struct KeyBinding {
    SDL_Keycode keycode;
    SDL_Keymod modifiers;
    KeyAction action;
    std::string description;
    std::string category;

    KeyBinding() = default;
    KeyBinding(SDL_Keycode key, SDL_Keymod mods, KeyAction act, const std::string& desc, const std::string& cat);

    bool operator==(const KeyBinding& other) const;
    bool operator!=(const KeyBinding& other) const;
};

/**
 * @brief Centralized key binding management system
 *
 * Provides a sophisticated system for managing keyboard shortcuts with support for:
 * - Context-sensitive bindings
 * - Dynamic rebinding
 * - Configuration persistence
 * - Category organization
 * - Modifier key support
 */
class KeyBindingManager {
public:
    using ActionCallback = std::function<void()>;

    KeyBindingManager();
    ~KeyBindingManager() = default;

    // Core functionality
    void registerAction(KeyAction action, ActionCallback callback);
    void registerBinding(const KeyBinding& binding);
    bool handleKey(SDL_Event* event);

    // Binding management
    void rebindKey(KeyAction action, SDL_Keycode keycode, SDL_Keymod modifiers);
    void clearBinding(KeyAction action);
    void loadDefaultBindings();

    // Context management
    void setContext(const std::string& context);
    std::string getCurrentContext() const;

    // Configuration persistence
    void saveToConfig(const std::string& configPath);
    void loadFromConfig(const std::string& configPath);

    // Query methods
    std::vector<std::string> getCategories() const;
    std::vector<KeyBinding> getBindingsByCategory(const std::string& category) const;
    std::string getBindingDescription(KeyAction action) const;
    std::string getKeyDisplayString(SDL_Keycode keycode, SDL_Keymod modifiers) const;
    std::map<KeyAction, std::string> getBindingsWithDisplayStrings(const std::string& category) const;

private:
    // Internal data structures
    std::map<KeyAction, KeyBinding> _bindings;
    std::map<KeyAction, ActionCallback> _actionCallbacks;
    std::map<std::pair<SDL_Keycode, SDL_Keymod>, KeyAction> _keyToAction;
    std::string _currentContext;

    // Helper methods
    bool modifiersMatch(SDL_Keymod current, SDL_Keymod required) const;
    void rebuildKeyToActionMap();
    std::string keyToString(SDL_Keycode keycode) const;
    std::string modifiersToString(SDL_Keymod modifiers) const;

    // Default binding setup
    void setupMixManagementBindings();
    void setupVisualizerBindings();
    void setupUIBindings();
    void setupApplicationBindings();
    void setupAudioBindings();
};

}  // namespace AutoVibez::Core
