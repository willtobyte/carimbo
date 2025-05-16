#include "audiodevice.hpp"

using namespace audio;

audiodevice::audiodevice()
    : _device(nullptr, [](ALCdevice *device) {
        if (device) {
          alcCloseDevice(device);
        }
      }),
      _context(nullptr, [](ALCcontext *context) {
        if (context) {
          alcMakeContextCurrent(nullptr);
          alcDestroyContext(context);
        }
      }) {
  _device.reset(alcOpenDevice(nullptr));
  if (!_device || alcGetError(_device.get()) != ALC_NO_ERROR) [[unlikely]] {
    throw std::runtime_error("[audiodevice] failed to open audio device");
  }

  _context.reset(alcCreateContext(_device.get(), nullptr));
  if (!_context || alcGetError(_device.get()) != ALC_NO_ERROR) [[unlikely]] {
    throw std::runtime_error("[audiodevice] failed to create audio context");
  }

  alcMakeContextCurrent(_context.get());
}
