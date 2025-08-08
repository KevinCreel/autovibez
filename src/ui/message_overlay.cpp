#include "message_overlay.hpp"

#include <backends/imgui_impl_opengl2.h>
#include <backends/imgui_impl_sdl2.h>
#include <imgui.h>

#include "constants.hpp"
#include "setup.hpp"

namespace AutoVibez {
namespace UI {

MessageOverlay::MessageOverlay() {}

MessageOverlay::~MessageOverlay() {
    if (_visible && _imguiReady) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
}

void MessageOverlay::init(SDL_Window* window, SDL_GLContext glContext) {
    if (_initialized) {
        return;  // Already initialized
    }

    _window = window;
    _glContext = glContext;

    // Get initial window size
    int width, height;
    SDL_GetWindowSize(_window, &width, &height);
    _windowWidth = width;
    _windowHeight = height;

    _initialized = true;
}

void MessageOverlay::render() {
    if (!_visible) {
        return;
    }

    // Update animation state
    updateAnimation();

    // Check if message should be hidden
    auto now = std::chrono::steady_clock::now();
    if (now >= _endTime) {
        _visible = false;
        return;
    }

    // Lazy initialize ImGui on first render
    if (!_imguiReady) {
        initializeImGui();
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Render the message box
    renderMessageBox();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void MessageOverlay::showMessage(const std::string& content, std::chrono::milliseconds duration) {
    MessageConfig config = getDefaultConfig();
    config.content = content;
    config.duration = duration;
    showMessage(config);
}

void MessageOverlay::showMessage(const MessageConfig& config) {
    _currentConfig = config;

    auto now = std::chrono::steady_clock::now();
    _startTime = now;
    _fadeInEndTime = now + _currentConfig.fadeInTime;
    _fadeOutStartTime = now + _currentConfig.duration - _currentConfig.fadeOutTime;
    _endTime = now + _currentConfig.duration;

    _visible = true;
    _currentAlpha = 0.0f;
}

void MessageOverlay::hideMessage() {
    _visible = false;
    _currentAlpha = 0.0f;
}

bool MessageOverlay::isVisible() const {
    return _visible;
}

bool MessageOverlay::isImGuiReady() const {
    return _imguiReady;
}

void MessageOverlay::setWindowSize(int width, int height) {
    _windowWidth = width;
    _windowHeight = height;
}

MessageOverlay::MessageConfig MessageOverlay::getDefaultConfig() {
    return MessageConfig{.content = "",
                         .duration = std::chrono::milliseconds(20000),  // 20 seconds default
                         .fadeInTime = std::chrono::milliseconds(300),
                         .fadeOutTime = std::chrono::milliseconds(300),
                         .backgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),  // Completely transparent
                         .textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                         .borderColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),  // No border
                         .cornerRadius = 0.0f,
                         .padding = 0.0f,
                         .showBorder = false,
                         .centerText = true,
                         .maxWidth = 0,  // No max width constraint
                         .useSlideAnimation = true,
                         .slideDistance = 50.0f};
}

MessageOverlay::MessageConfig MessageOverlay::getSuccessConfig() {
    MessageConfig config = getDefaultConfig();
    config.textColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);  // Bright green text
    return config;
}

MessageOverlay::MessageConfig MessageOverlay::getErrorConfig() {
    MessageConfig config = getDefaultConfig();
    config.textColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);  // Bright red text
    return config;
}

MessageOverlay::MessageConfig MessageOverlay::getWarningConfig() {
    MessageConfig config = getDefaultConfig();
    config.textColor = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);  // Bright yellow text
    return config;
}

MessageOverlay::MessageConfig MessageOverlay::getInfoConfig() {
    MessageConfig config = getDefaultConfig();
    config.textColor = ImVec4(0.2f, 0.8f, 1.0f, 1.0f);  // Bright cyan text
    return config;
}

void MessageOverlay::initializeImGui() {
    if (_imguiReady) {
        return;
    }

    // Use centralized ImGui initialization
    if (AutoVibez::UI::ImGuiManager::initialize(_window, _glContext)) {
        // Configure ImGui for this overlay
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // Add default font
        io.Fonts->AddFontDefault();
        io.FontGlobalScale = 1.0f;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Explicitly create the font texture
        ImGui_ImplOpenGL2_CreateFontsTexture();

        _imguiReady = true;
    }
}

void MessageOverlay::updateAnimation() {
    auto now = std::chrono::steady_clock::now();

    if (now <= _fadeInEndTime) {
        // Fade in phase
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime);
        float progress = static_cast<float>(elapsed.count()) / static_cast<float>(_currentConfig.fadeInTime.count());
        _currentAlpha = std::min(1.0f, progress);
    } else if (now >= _fadeOutStartTime) {
        // Fade out phase
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _fadeOutStartTime);
        float progress = static_cast<float>(elapsed.count()) / static_cast<float>(_currentConfig.fadeOutTime.count());
        _currentAlpha = std::max(0.0f, 1.0f - progress);
    } else {
        // Fully visible phase
        _currentAlpha = 1.0f;
    }
}

float MessageOverlay::calculateCurrentAlpha() {
    return _currentAlpha;
}

void MessageOverlay::renderMessageBox() {
    // Calculate font size based on window size
    float fontSize = std::max(24.0f, std::min(72.0f, _windowHeight * 0.04f));  // 4% of window height, min 24, max 72

    // Use ImGui's built-in overlay features
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                   ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs |
                                   ImGuiWindowFlags_NoScrollbar;

    // Position window at top center using ImGui's viewport features
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float x = workPos.x + workSize.x * 0.5f;
    float y = workPos.y + 50.0f;  // 50px from top

    // Add slide animation (slide down from top)
    if (_currentConfig.useSlideAnimation) {
        float slideOffset = (1.0f - _currentAlpha) * _currentConfig.slideDistance;
        y -= slideOffset;  // Slide down from above
    }

    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always, ImVec2(0.5f, 0.0f));

    // Set completely transparent background using ImGui's alpha system
    ImGui::SetNextWindowBgAlpha(0.0f);

    // Use ImGui's built-in text rendering with proper styling
    if (ImGui::Begin("##MessageOverlay", nullptr, windowFlags)) {
        // Set large font using ImGui's font system
        ImGui::SetWindowFontScale(fontSize / 13.0f);

        // Apply alpha to text color using ImGui's color system
        ImVec4 textColor = _currentConfig.textColor;
        textColor.w *= _currentAlpha;
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);

        // Use ImGui's built-in text centering with proper spacing
        ImVec2 textSize = ImGui::CalcTextSize(_currentConfig.content.c_str());
        float centerX = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;
        ImGui::SetCursorPosX(centerX);

        // Add padding using ImGui's spacing system
        ImGui::Spacing();

        // Render the text using ImGui's text rendering
        ImGui::Text("%s", _currentConfig.content.c_str());

        // Add bottom spacing
        ImGui::Spacing();

        ImGui::PopStyleColor();

        // Reset font scale
        ImGui::SetWindowFontScale(1.0f);
    }
    ImGui::End();
}

ImVec2 MessageOverlay::calculateMessagePosition() {
    // Use ImGui's viewport system for proper positioning
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;

    float x = workPos.x + workSize.x * 0.5f;
    float y = workPos.y + 50.0f;  // 50px from top

    return ImVec2(x, y);
}

}  // namespace UI
}  // namespace AutoVibez
