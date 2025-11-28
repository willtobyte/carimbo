#include "audiodevice.hpp"

audiodevice::audiodevice() {
  _device = unwrap(
    std::unique_ptr<ALCdevice, ALC_Deleter>(alcOpenDevice(nullptr)),
    "failed to open audio device"
  );

  _context = unwrap(
    std::unique_ptr<ALCcontext, ALC_Deleter>(alcCreateContext(_device.get(), nullptr)),
    "failed to create audio context"
  );

  [[maybe_unused]] const auto result = alcMakeContextCurrent(_context.get());
  assert(result != ALC_FALSE && "[alcMakeContextCurrent] failed to set current context");
}
