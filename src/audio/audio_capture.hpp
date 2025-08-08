#pragma once

namespace AutoVibez::Audio {

namespace AutoVibez::Core {
class AutoVibezApp;
}

void audioInputCallbackF32(void* userData, const float* buffer, int len);

}  // namespace AutoVibez::Audio
