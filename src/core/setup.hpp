#pragma once

#include <string>

#include "autovibez_app.hpp"

namespace AutoVibez {
namespace Core {
class AutoVibezApp;
}
}  // namespace AutoVibez

void debugGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
             const void *userParam);

std::string getConfigDirectory();
std::string getConfigFilePath(std::string datadir_path);
std::string findConfigFile();
void seedRand();
void initGL();
void dumpOpenGLInfo();
void initStereoscopicView(SDL_Window *win);
void enableGLDebugOutput();
void testAllPresets(AutoVibez::Core::AutoVibezApp *app);
AutoVibez::Core::AutoVibezApp *setupSDLApp();
int64_t startUnlockedFPSCounter();
void advanceUnlockedFPSCounterFrame(int64_t startFrame);
