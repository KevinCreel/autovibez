#pragma once

#include <string>

#include "autovibez_app.hpp"

namespace AutoVibez::Core {
class AutoVibezApp;
}

void debugGL();

std::string getConfigDirectory();
std::string getConfigFilePath(std::string datadir_path);
std::string findConfigFile();
void seedRand();
void initGL();
void enableGLDebugOutput();
AutoVibez::Core::AutoVibezApp *setupSDLApp();
