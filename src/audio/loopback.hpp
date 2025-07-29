

#ifndef loopback_hpp
#define loopback_hpp

#include "autovibez_app.hpp"

class AutoVibezApp;

bool initLoopback();
void configureLoopback(AutoVibezApp *app);
bool processLoopbackFrame(AutoVibezApp *app);
bool cleanupLoopback();

#endif /* loopback_hpp */
