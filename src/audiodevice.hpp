#pragma once

#include "common.hpp"

namespace audio {

class audiodevice final {
public:
  audiodevice();
  ~audiodevice() = default;

  audiodevice(const audiodevice&) = delete;
  audiodevice& operator=(const audiodevice&) = delete;
  audiodevice(audiodevice&&) = delete;
  audiodevice& operator=(audiodevice&&) = delete;

private:
  std::unique_ptr<ALCdevice, ALC_Deleter> _device;
  std::unique_ptr<ALCcontext, ALC_Deleter> _context;
};
}
