#include "audiodevice.hpp"

using namespace audio;

audiodevice::audiodevice()
    : device(nullptr, [](ALCdevice *device) {
        if (device) {
          alcCloseDevice(device);
        }
      }),
      context(nullptr, [](ALCcontext *context) {
        if (context) {
          alcMakeContextCurrent(nullptr);
          alcDestroyContext(context);
        }
      }) {
  device.reset(alcOpenDevice(nullptr));
  if (!device || alcGetError(device.get()) != ALC_NO_ERROR) [[unlikely]] {
    panic("[audiodevice] failed to open ALC device");
  }

  context.reset(alcCreateContext(device.get(), nullptr));
  if (!context || alcGetError(device.get()) != ALC_NO_ERROR) [[unlikely]] {
    panic("[audiodevice] failed to create ALC context");
  }

  alcMakeContextCurrent(context.get());
}
