#pragma once

#include "common.hpp"

namespace audio {
class audiodevice final {
public:
  audiodevice();
  ~audiodevice() noexcept = default;

private:
  std::unique_ptr<ALCdevice, void (*)(ALCdevice*)> _device;
  std::unique_ptr<ALCcontext, void (*)(ALCcontext*)> _context;
};
}
