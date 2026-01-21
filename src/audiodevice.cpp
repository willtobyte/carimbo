#include "audiodevice.hpp"

audiodevice::audiodevice() {
  static const auto device = unwrap(
    std::unique_ptr<ALCdevice, ALC_Deleter>(alcOpenDevice(nullptr)),
    "failed to open audio device"
  );

  static const auto context = unwrap(
    std::unique_ptr<ALCcontext, ALC_Deleter>(alcCreateContext(device.get(), nullptr)),
    "failed to create audio context"
  );

  [[maybe_unused]] const auto result = alcMakeContextCurrent(context.get());
  assert(result != ALC_FALSE && "[alcMakeContextCurrent] failed to set current context");

  std::at_quick_exit([] {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context.get());
    alcCloseDevice(device.get());
  });
}
