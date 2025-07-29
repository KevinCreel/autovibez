#include "app_state.hpp"
#include <sstream>

void AppState::reset() {
    // Reset window state
    window = WindowState{};
    
    // Reset audio state
    audio = AudioState{};
    
    // Reset UI state
    ui = UIState{};
    
    // Reset mix state
    mix = MixState{};
    
    // Reset preset state
    preset = PresetState{};
}

std::string AppState::getStateSummary() const {
    std::ostringstream oss;
    
    oss << "=== App State Summary ===\n";
    
    // Window state
    oss << "Window: " << window.width << "x" << window.height;
    oss << " (fullscreen: " << (window.isFullscreen ? "yes" : "no") << ")";
    oss << " (stretched: " << (window.isStretched ? "yes" : "no") << ")\n";
    
    // Audio state
    oss << "Audio: device " << audio.currentDevice << "/" << audio.deviceCount;
    oss << " (capturing: " << (audio.isCapturing ? "yes" : "no") << ")";
    oss << " (fake: " << (audio.isFakeAudio ? "yes" : "no") << ")\n";
    
    // UI state
    oss << "UI: help=" << (ui.showHelp ? "on" : "off");
    oss << " fps=" << (ui.showFps ? "on" : "off");
    oss << " mouse=" << (ui.mouseDown ? "down" : "up");
    oss << " keymod=" << (ui.keymod ? "on" : "off") << "\n";
    
    // Mix state
    oss << "Mix: initialized=" << (mix.managerInitialized ? "yes" : "no");
    oss << " status=" << (mix.showMixStatus ? "on" : "off");
    oss << " time=" << mix.mixStatusDisplayTime;
    oss << " genre='" << mix.currentGenre << "'\n";
    
    // Preset state
    oss << "Preset: '" << preset.currentPresetName << "'";
    oss << " (playing: " << (preset.isPlaying ? "yes" : "no") << ")";
    oss << " (paused: " << (preset.isPaused ? "yes" : "no") << ")\n";
    
    return oss.str();
} 