#pragma once

#include "common.hpp"

namespace audio {
class audiodevice {
public:
  audiodevice();
  ~audiodevice() noexcept = default;

private:
  std::unique_ptr<ALCdevice, void (*)(ALCdevice *)> device;
  std::unique_ptr<ALCcontext, void (*)(ALCcontext *)> context;
};
}
