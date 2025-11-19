#include "audiodevice.hpp"

using namespace audio;

audiodevice::audiodevice() {
  _device = unwrap(
    std::unique_ptr<ALCdevice, ALC_Deleter>(alcOpenDevice(nullptr)),
    "failed to open audio device"
  );

  _context = unwrap(
    std::unique_ptr<ALCcontext, ALC_Deleter>(alcCreateContext(_device.get(), nullptr)),
    "failed to create audio context"
  );

  if (const auto result = alcMakeContextCurrent(_context.get()); result == ALC_FALSE) [[unlikely]] {
    throw std::runtime_error("[alcMakeContextCurrent] failed to set current context");
  }
}
