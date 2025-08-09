#include "key_binding_manager.hpp"

#include <algorithm>
#include <sstream>

#include "utils/constants.hpp"

namespace AutoVibez::Core {

// KeyBinding implementation
KeyBinding::KeyBinding(SDL_Keycode key, SDL_Keymod mods, KeyAction act, const std::string& desc, const std::string& cat)
    : keycode(key), modifiers(mods), action(act), description(desc), category(cat) {}

bool KeyBinding::operator==(const KeyBinding& other) const {
    return keycode == other.keycode && modifiers == other.modifiers && action == other.action;
}

bool KeyBinding::operator!=(const KeyBinding& other) const {
    return !(*this == other);
}

// KeyBindingManager implementation
KeyBindingManager::KeyBindingManager() : _currentContext("default") {
    loadDefaultBindings();
}

void KeyBindingManager::registerAction(KeyAction action, ActionCallback callback) {
    _actionCallbacks[action] = callback;
}

void KeyBindingManager::registerBinding(const KeyBinding& binding) {
    _bindings[binding.action] = binding;
    rebuildKeyToActionMap();
}

bool KeyBindingManager::handleKey(SDL_Event* event) {
    if (!event || event->type != SDL_KEYDOWN) {
        return false;
    }

    SDL_Keycode keycode = event->key.keysym.sym;
    SDL_Keymod modifiers = static_cast<SDL_Keymod>(event->key.keysym.mod);

    // Find matching action
    for (const auto& [action, binding] : _bindings) {
        if (binding.keycode == keycode && modifiersMatch(modifiers, binding.modifiers)) {
            auto callbackIt = _actionCallbacks.find(action);
            if (callbackIt != _actionCallbacks.end()) {
                callbackIt->second();
            }
            return true;  // Key binding found and recognized, regardless of callback existence
        }
    }

    return false;
}

void KeyBindingManager::rebindKey(KeyAction action, SDL_Keycode keycode, SDL_Keymod modifiers) {
    auto it = _bindings.find(action);
    if (it != _bindings.end()) {
        it->second.keycode = keycode;
        it->second.modifiers = modifiers;
        rebuildKeyToActionMap();
    }
}

void KeyBindingManager::clearBinding(KeyAction action) {
    _bindings.erase(action);
    rebuildKeyToActionMap();
}

void KeyBindingManager::setContext(const std::string& context) {
    _currentContext = context;
}

std::string KeyBindingManager::getCurrentContext() const {
    return _currentContext;
}

void KeyBindingManager::saveToConfig(const std::string& configPath) {
    // Implementation would save bindings to configuration file
    // For now, this is a placeholder
}

void KeyBindingManager::loadFromConfig(const std::string& configPath) {
    // Implementation would load bindings from configuration file
    // For now, this is a placeholder
}

std::vector<std::string> KeyBindingManager::getCategories() const {
    std::vector<std::string> categories;
    for (const auto& [action, binding] : _bindings) {
        if (std::find(categories.begin(), categories.end(), binding.category) == categories.end()) {
            categories.push_back(binding.category);
        }
    }
    std::sort(categories.begin(), categories.end());
    return categories;
}

std::vector<KeyBinding> KeyBindingManager::getBindingsByCategory(const std::string& category) const {
    std::vector<KeyBinding> result;
    for (const auto& [action, binding] : _bindings) {
        if (binding.category == category) {
            result.push_back(binding);
        }
    }
    return result;
}

std::string KeyBindingManager::getBindingDescription(KeyAction action) const {
    auto it = _bindings.find(action);
    return (it != _bindings.end()) ? it->second.description : "";
}

std::string KeyBindingManager::getKeyDisplayString(SDL_Keycode keycode, SDL_Keymod modifiers) const {
    std::string result;

    if (modifiers & KMOD_CTRL)
        result += "Ctrl+";
    if (modifiers & KMOD_ALT)
        result += "Alt+";
    if (modifiers & KMOD_SHIFT)
        result += "Shift+";
    if (modifiers & KMOD_GUI)
        result += "Meta+";

    result += keyToString(keycode);
    return result;
}

std::map<KeyAction, std::string> KeyBindingManager::getBindingsWithDisplayStrings(const std::string& category) const {
    std::map<KeyAction, std::string> result;
    for (const auto& [action, binding] : _bindings) {
        if (binding.category == category) {
            result[action] = getKeyDisplayString(binding.keycode, binding.modifiers);
        }
    }
    return result;
}

bool KeyBindingManager::modifiersMatch(SDL_Keymod current, SDL_Keymod required) const {
    // Mask out caps lock, num lock, etc.
    SDL_Keymod relevantMods = static_cast<SDL_Keymod>(KMOD_CTRL | KMOD_ALT | KMOD_SHIFT | KMOD_GUI);
    return (current & relevantMods) == (required & relevantMods);
}

void KeyBindingManager::rebuildKeyToActionMap() {
    _keyToAction.clear();
    for (const auto& [action, binding] : _bindings) {
        _keyToAction[{binding.keycode, binding.modifiers}] = action;
    }
}

std::string KeyBindingManager::keyToString(SDL_Keycode keycode) const {
    switch (keycode) {
        case SDLK_SPACE:
            return "Space";
        case SDLK_RETURN:
            return "Enter";
        case SDLK_ESCAPE:
            return "Escape";
        case SDLK_TAB:
            return "Tab";
        case SDLK_BACKSPACE:
            return "Backspace";
        case SDLK_DELETE:
            return "Delete";
        case SDLK_LEFT:
            return "Left";
        case SDLK_RIGHT:
            return "Right";
        case SDLK_UP:
            return "Up";
        case SDLK_DOWN:
            return "Down";
        case SDLK_F1:
            return "F1";
        case SDLK_F2:
            return "F2";
        case SDLK_F3:
            return "F3";
        case SDLK_F4:
            return "F4";
        case SDLK_F5:
            return "F5";
        case SDLK_F6:
            return "F6";
        case SDLK_F7:
            return "F7";
        case SDLK_F8:
            return "F8";
        case SDLK_F9:
            return "F9";
        case SDLK_F10:
            return "F10";
        case SDLK_F11:
            return "F11";
        case SDLK_F12:
            return "F12";
        default:
            if (keycode >= SDLK_a && keycode <= SDLK_z) {
                return std::string(1, static_cast<char>(keycode - SDLK_a + 'A'));
            }
            if (keycode >= SDLK_0 && keycode <= SDLK_9) {
                return std::string(1, static_cast<char>(keycode));
            }
            return "Unknown";
    }
}

std::string KeyBindingManager::modifiersToString(SDL_Keymod modifiers) const {
    std::string result;
    if (modifiers & KMOD_CTRL)
        result += "Ctrl+";
    if (modifiers & KMOD_ALT)
        result += "Alt+";
    if (modifiers & KMOD_SHIFT)
        result += "Shift+";
    if (modifiers & KMOD_GUI)
        result += "Meta+";
    return result;
}

void KeyBindingManager::loadDefaultBindings() {
    setupMixManagementBindings();
    setupVisualizerBindings();
    setupUIBindings();
    setupApplicationBindings();
    setupAudioBindings();
    rebuildKeyToActionMap();
}

void KeyBindingManager::setupMixManagementBindings() {
    registerBinding({SDLK_LEFT, KMOD_NONE, KeyAction::PREVIOUS_MIX, "Previous mix", "MIX MANAGEMENT"});
    registerBinding({SDLK_RIGHT, KMOD_NONE, KeyAction::NEXT_MIX, "Next mix", "MIX MANAGEMENT"});
    registerBinding({SDLK_f, KMOD_NONE, KeyAction::TOGGLE_FAVORITE, "Toggle favorite", "MIX MANAGEMENT"});
    registerBinding({SDLK_d, KMOD_NONE, KeyAction::SOFT_DELETE_MIX, "Delete current mix", "MIX MANAGEMENT"});
    registerBinding({SDLK_i, KMOD_NONE, KeyAction::SHOW_MIX_INFO, "Show current mix info", "MIX MANAGEMENT"});
    registerBinding(
        {SDLK_l, KMOD_NONE, KeyAction::TOGGLE_MIX_TABLE_FILTER, "Toggle favorites filter", "MIX MANAGEMENT"});
    registerBinding(
        {SDLK_g, KMOD_NONE, KeyAction::RANDOM_MIX_CURRENT_GENRE, "Play random mix in current genre", "MIX MANAGEMENT"});
    registerBinding({SDLK_g, KMOD_SHIFT, KeyAction::RANDOM_GENRE_AND_MIX, "Switch to random genre", "MIX MANAGEMENT"});
    registerBinding({SDLK_SPACE, KMOD_NONE, KeyAction::PAUSE_RESUME_MIX, "Pause/Resume playbook", "MIX MANAGEMENT"});
}

void KeyBindingManager::setupVisualizerBindings() {
    registerBinding(
        {SDLK_h, KMOD_NONE, KeyAction::TOGGLE_HELP_OVERLAY, "Toggle this help overlay", "VISUALIZER CONTROLS"});
    registerBinding(
        {SDLK_F11, KMOD_NONE, KeyAction::TOGGLE_FULLSCREEN, "Toggle fullscreen mode", "VISUALIZER CONTROLS"});
    registerBinding({SDLK_r, KMOD_NONE, KeyAction::RANDOM_PRESET, "Load random preset", "VISUALIZER CONTROLS"});
    registerBinding(
        {SDLK_LEFTBRACKET, KMOD_NONE, KeyAction::PREVIOUS_PRESET_BRACKET, "Previous preset", "VISUALIZER CONTROLS"});
    registerBinding(
        {SDLK_RIGHTBRACKET, KMOD_NONE, KeyAction::NEXT_PRESET_BRACKET, "Next preset", "VISUALIZER CONTROLS"});
    registerBinding({SDLK_PLUS, KMOD_NONE, KeyAction::INCREASE_BEAT_SENSITIVITY, "Increase beat sensitivity",
                     "VISUALIZER CONTROLS"});
    registerBinding({SDLK_EQUALS, KMOD_NONE, KeyAction::INCREASE_BEAT_SENSITIVITY, "Increase beat sensitivity",
                     "VISUALIZER CONTROLS"});
    registerBinding({SDLK_MINUS, KMOD_NONE, KeyAction::DECREASE_BEAT_SENSITIVITY, "Decrease beat sensitivity",
                     "VISUALIZER CONTROLS"});
}

void KeyBindingManager::setupUIBindings() {
    // UI Controls are now handled in other categories to match help overlay exactly
}

void KeyBindingManager::setupApplicationBindings() {
    registerBinding({SDLK_q, KMOD_CTRL, KeyAction::QUIT_WITH_MODIFIER, "Quit application", "APPLICATION"});
}

void KeyBindingManager::setupAudioBindings() {
    registerBinding({SDLK_m, KMOD_NONE, KeyAction::TOGGLE_MUTE, "Mute/Unmute audio", "AUDIO CONTROLS"});
    registerBinding({SDLK_UP, KMOD_NONE, KeyAction::VOLUME_UP, "Volume up", "AUDIO CONTROLS"});
    registerBinding({SDLK_DOWN, KMOD_NONE, KeyAction::VOLUME_DOWN, "Volume down", "AUDIO CONTROLS"});
    registerBinding(
        {SDLK_TAB, KMOD_NONE, KeyAction::CYCLE_AUDIO_DEVICE, "Cycle through audio devices", "AUDIO CONTROLS"});
}

}  // namespace AutoVibez::Core
