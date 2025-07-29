

#ifndef loopback_hpp
#define loopback_hpp

#include "autovibez_app.hpp"

namespace AutoVibez { namespace Core { class AutoVibezApp; } }

namespace AutoVibez {
namespace Audio {

bool initLoopback();
void configureLoopback(Core::AutoVibezApp *app);
bool processLoopbackFrame(Core::AutoVibezApp *app);
bool cleanupLoopback();

} // namespace Audio
} // namespace AutoVibez

#endif /* loopback_hpp */
