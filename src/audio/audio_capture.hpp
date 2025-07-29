#ifndef audio_capture_hpp
#define audio_capture_hpp

namespace AutoVibez {
namespace Audio {

namespace AutoVibez { namespace Core { class AutoVibezApp; } }

void audioInputCallbackF32(void* userData, const float* buffer, int len);

} // namespace Audio
} // namespace AutoVibez

#endif /* audio_capture_hpp */
