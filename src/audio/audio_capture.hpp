#pragma once

namespace AutoVibez {
namespace Audio {

namespace AutoVibez {
namespace Core {
class AutoVibezApp;
}
}  // namespace AutoVibez

void audioInputCallbackF32(void* userData, const float* buffer, int len);

}  // namespace Audio
}  // namespace AutoVibez
