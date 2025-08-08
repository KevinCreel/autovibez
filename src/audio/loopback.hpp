

#pragma once

#include "autovibez_app.hpp"

namespace AutoVibez::Core {
class AutoVibezApp;
}

namespace AutoVibez::Audio {

bool initLoopback();
void configureLoopback(Core::AutoVibezApp *app);
bool processLoopbackFrame(Core::AutoVibezApp *app);
bool cleanupLoopback();

}  // namespace AutoVibez::Audio
