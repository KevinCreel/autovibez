#include "help_overlay.hpp"
#include "setup.hpp"
#include "console_output.hpp"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl2.h>
#include <iostream>

namespace AutoVibez {
namespace UI {

HelpOverlay::HelpOverlay() {
}

HelpOverlay::~HelpOverlay() {
    if (_visible && _imguiReady) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
    
    // Free cursors
    if (_blankCursor) {
        SDL_FreeCursor(_blankCursor);
        _blankCursor = nullptr;
    }
    // Note: Don't free _originalCursor as it's managed by SDL
}

void HelpOverlay::init(SDL_Window* window, SDL_GLContext glContext) {
    if (_initialized) {
        return; // Already initialized
    }
    
    _window = window;
    _glContext = glContext;
    
    // Create a blank cursor (1x1 transparent pixel)
    Uint8 blankData[4] = {0, 0, 0, 0}; // Transparent
    Uint8 blankMask[4] = {0, 0, 0, 0}; // Transparent
    _blankCursor = SDL_CreateCursor(blankData, blankMask, 1, 1, 0, 0);
    
    // Store the original cursor
    _originalCursor = SDL_GetCursor();
    
    _initialized = true;
    // Note: ImGui will be initialized on first render to avoid conflicts
}

void HelpOverlay::render() {
    if (!_visible) return;
    
    // For now, let's try a simpler approach - just render a basic overlay
    // without complex ImGui setup to avoid rendering conflicts
    
    // Lazy initialize ImGui on first render
    if (!_imguiReady) {
        // Ensure we have the OpenGL context
        SDL_GL_MakeCurrent(_window, _glContext);
        
        // Setup Dear ImGui context with minimal configuration
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        // Disable font atlas completely to avoid rendering issues
        io.Fonts->AddFontDefault();
        io.FontGlobalScale = 1.0f;
        
        // Set INI file path to user config directory
        std::string configDir = getConfigDirectory();
        if (!configDir.empty()) {
            std::string iniPath = configDir + "/imgui.ini";
            io.IniFilename = strdup(iniPath.c_str());
        }
        
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        
        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
        ImGui_ImplOpenGL2_Init();
        
        _imguiReady = true;
    }
    
    // Use a very simple rendering approach
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    // Get window size
    int windowWidth, windowHeight;
    SDL_GetWindowSize(_window, &windowWidth, &windowHeight);
    
    // Set up the main window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
    ImGui::SetNextWindowBgAlpha(1.0f); // Make completely opaque to hide cursor
    
    ImGui::Begin("AutoVibez Help", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | 
                 ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);
    
    // Use very basic font rendering with no scaling
    ImGui::SetWindowFontScale(1.0f);
    
    // Title with gradient-like effect
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
    ImGui::TextUnformatted("AUTOVIBEZ CONTROLS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Mix Management Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
    ImGui::TextUnformatted("MIX MANAGEMENT");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::TextUnformatted("N     - Play next mix");
    ImGui::TextUnformatted("F     - Toggle favorite");
    ImGui::TextUnformatted("V     - List favorite mixes");
    ImGui::TextUnformatted("L     - List available mixes");
    ImGui::TextUnformatted("G     - Play random mix in current genre");
    ImGui::TextUnformatted("Shift+G - Switch to random genre");
    ImGui::TextUnformatted("Ctrl+G  - Show available genres");
    ImGui::TextUnformatted("SPACE  - Load random mix");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Audio Controls Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
    ImGui::TextUnformatted("AUDIO CONTROLS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::TextUnformatted("P     - Pause/Resume playback");
    ImGui::TextUnformatted("Up/Down - Volume up/down");
    ImGui::TextUnformatted("Tab   - Cycle through audio devices");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Visualizer Controls Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 1.0f, 1.0f));
    ImGui::TextUnformatted("VISUALIZER CONTROLS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::TextUnformatted("H     - Toggle this help overlay");
    ImGui::TextUnformatted("F11   - Toggle fullscreen mode");
    ImGui::TextUnformatted("R     - Load random preset");
    ImGui::TextUnformatted("[ / ]  - Previous/Next preset");
    ImGui::TextUnformatted("B / J  - Increase/Decrease beat sensitivity");
    ImGui::TextUnformatted("Mouse Wheel - Next/Prev preset");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Application Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    ImGui::TextUnformatted("APPLICATION");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::TextUnformatted("Ctrl+Q - Quit application");
    ImGui::PopStyleColor();
    
    ImGui::End();
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    
    // Minimal cleanup
    glDisable(GL_BLEND);
}

void HelpOverlay::toggle() {
    _visible = !_visible;
    
    if (_visible) {
        // Only hide cursor in fullscreen mode
        if (_isFullscreen) {
            _cursorWasVisible = SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
    } else {
        // Only restore cursor if we're not in fullscreen mode
        if (!_isFullscreen) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            if (_cursorWasVisible) {
                SDL_ShowCursor(SDL_ENABLE);
            }
        }
        // In fullscreen mode, keep relative mouse mode enabled (cursor hidden)
    }
}

void HelpOverlay::setFullscreenState(bool isFullscreen) {
    _isFullscreen = isFullscreen;
}

void HelpOverlay::setCursorVisibility(bool visible) {
    if (visible) {
        SDL_SetCursor(_originalCursor);
    } else {
        SDL_SetCursor(_blankCursor);
    }
}

void HelpOverlay::rebuildFontAtlas() {
    if (_imguiReady) {
        // Ensure we have the OpenGL context
        SDL_GL_MakeCurrent(_window, _glContext);
        
        // Rebuild the font atlas
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();
        io.Fonts->AddFontDefault();
        
        // Build the font atlas
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        
        // This forces ImGui to recreate its font texture
        ImGui_ImplOpenGL2_DestroyFontsTexture();
        ImGui_ImplOpenGL2_CreateFontsTexture();
    }
}

} // namespace UI
} // namespace AutoVibez 