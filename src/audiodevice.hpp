#pragma once

#include "common.hpp"

namespace audio {
class audiodevice final {
public:
  audiodevice();
  ~audiodevice() = default;

private:
  std::unique_ptr<ALCdevice, void (*)(ALCdevice *)> device;
  std::unique_ptr<ALCcontext, void (*)(ALCcontext *)> context;
};
}
