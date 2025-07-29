#include "simple_ui.hpp"
#include "setup.hpp"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl2.h>
#include <iostream>

SimpleUI::SimpleUI() {
}

SimpleUI::~SimpleUI() {
    if (_visible) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
}

void SimpleUI::init(SDL_Window* window, SDL_GLContext glContext) {
    _window = window;
    _glContext = glContext;
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Set INI file path to user config directory
    std::string configDir = getConfigDirectory();
    if (!configDir.empty()) {
        std::string iniPath = configDir + "/imgui.ini";
        io.IniFilename = strdup(iniPath.c_str());
    }
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL2_Init();
}

void SimpleUI::render() {
    if (!_visible) return;
    
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
    ImGui::SetNextWindowBgAlpha(0.85f);
    
    ImGui::Begin("AutoVibez Help", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | 
                 ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);
    
    // Set larger font size
    ImGui::SetWindowFontScale(1.3f);
    
    // Title with gradient-like effect
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 1.0f, 1.0f));
    ImGui::Text("AUTOVIBEZ CONTROLS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Mix Management Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
    ImGui::Text("MIX MANAGEMENT");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::Text("N     - Play next mix");
    ImGui::Text("F     - Toggle favorite");
    ImGui::Text("V     - List favorite mixes");
    ImGui::Text("L     - List available mixes");
    ImGui::Text("G     - Play random mix in current genre");
    ImGui::Text("Shift+G - Switch to random genre");
    ImGui::Text("Ctrl+G  - Show available genres");
    ImGui::Text("SPACE  - Load random mix");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Audio Controls Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
    ImGui::Text("AUDIO CONTROLS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::Text("P     - Pause/Resume playback");
    ImGui::Text("Up/Down - Volume up/down");
    ImGui::Text("Tab   - Cycle through audio devices");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Visualizer Controls Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 1.0f, 1.0f));
    ImGui::Text("VISUALIZER CONTROLS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::Text("H     - Toggle this help overlay");
    ImGui::Text("F11   - Toggle fullscreen mode");
    ImGui::Text("R     - Load random preset");
    ImGui::Text("[ / ]  - Previous/Next preset");
    ImGui::Text("B / J  - Increase/Decrease beat sensitivity");
    ImGui::Text("Mouse Wheel - Next/Prev preset");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Application Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    ImGui::Text("APPLICATION");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::Text("Ctrl+Q - Quit application");
    ImGui::PopStyleColor();
    
    ImGui::End();
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

void SimpleUI::toggle() {
    _visible = !_visible;
} 