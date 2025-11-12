#include "audiodevice.hpp"

using namespace audio;

audiodevice::audiodevice() : _device(nullptr), _context(nullptr) {
  _device.reset(alcOpenDevice(nullptr));

  if (!_device) [[unlikely]] {
    throw std::runtime_error("[alcOpenDevice] failed to open audio device");
  }

  if (const auto error = alcGetError(_device.get()); error != ALC_NO_ERROR) [[unlikely]] {
    throw std::runtime_error("[alcOpenDevice] failed to open audio device");
  }

  _context.reset(alcCreateContext(_device.get(), nullptr));

  if (!_context) [[unlikely]] {
    throw std::runtime_error("[alcCreateContext] failed to create audio context");
  }

  if (const auto error = alcGetError(_device.get()); error != ALC_NO_ERROR) [[unlikely]] {
    throw std::runtime_error("[alcCreateContext] failed to create audio context");
  }

  if (const auto result = alcMakeContextCurrent(_context.get()); result == ALC_FALSE) [[unlikely]] {
    throw std::runtime_error("[alcMakeContextCurrent] failed to set current context");
  }
}
