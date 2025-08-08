#pragma once

#include <SDL2/SDL.h>
#include <imgui.h>

namespace AutoVibez::UI {

/**
 * @brief Simple ImGui manager for centralized initialization only
 *
 * Manages ImGui initialization and backend setup. Each overlay handles
 * its own frame management independently.
 */
class ImGuiManager {
public:
    /**
     * @brief Initialize ImGui with SDL2 and OpenGL2 backends
     * @param window SDL window pointer
     * @param glContext OpenGL context
     * @return true if initialization successful
     */
    static bool initialize(SDL_Window* window, SDL_GLContext glContext);

    /**
     * @brief Check if ImGui is ready for use
     * @return true if ImGui is initialized and ready
     */
    static bool isReady();

    /**
     * @brief Shutdown ImGui and clean up resources
     */
    static void shutdown();

private:
    static bool _initialized;
    static SDL_Window* _window;
    static SDL_GLContext _glContext;
};

}  // namespace AutoVibez::UI
