#include "imgui_manager.hpp"

#include <backends/imgui_impl_opengl2.h>
#include <backends/imgui_impl_sdl2.h>

namespace AutoVibez::UI {

// Static member initialization
bool ImGuiManager::_initialized = false;
SDL_Window* ImGuiManager::_window = nullptr;
SDL_GLContext ImGuiManager::_glContext = nullptr;

bool ImGuiManager::initialize(SDL_Window* window, SDL_GLContext glContext) {
    if (_initialized) {
        return true;  // Already initialized
    }

    _window = window;
    _glContext = glContext;

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Configure ImGui IO to not capture arrow keys for navigation
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;  // Disable keyboard navigation

    // Setup Platform/Renderer backends
    if (!ImGui_ImplSDL2_InitForOpenGL(_window, _glContext)) {
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplOpenGL2_Init()) {
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    _initialized = true;
    return true;
}

bool ImGuiManager::isReady() {
    return _initialized;
}

void ImGuiManager::shutdown() {
    if (_initialized) {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        _initialized = false;
        _window = nullptr;
        _glContext = nullptr;
    }
}

}  // namespace AutoVibez::UI
