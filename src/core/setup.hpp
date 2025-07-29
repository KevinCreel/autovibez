#ifndef setup_hpp
#define setup_hpp

#include "autovibez_app.hpp"

#include <string>

namespace AutoVibez { namespace Core { class AutoVibezApp; } }

void debugGL(GLenum source,
               GLenum type,
               GLuint id,
               GLenum severity,
               GLsizei length,
               const GLchar* message,
             const void* userParam);

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

#endif /* setup_hpp */
