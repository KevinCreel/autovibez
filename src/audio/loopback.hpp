

#ifndef loopback_hpp
#define loopback_hpp

#include "autovibez_app.hpp"

namespace AutoVibez { namespace Core { class AutoVibezApp; } }

bool initLoopback();
void configureLoopback(AutoVibez::Core::AutoVibezApp *app);
bool processLoopbackFrame(AutoVibez::Core::AutoVibezApp *app);
bool cleanupLoopback();

#endif /* loopback_hpp */
