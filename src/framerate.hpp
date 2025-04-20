#pragma once

#include "common.hpp"

#include "loopable.hpp"

namespace framework {
class framerate : public loopable {
public:
  framerate() noexcept = default;
  virtual ~framerate() noexcept = default;

  void loop(float_t delta) noexcept override;

private:
  uint64_t _frames{0};
  uint64_t _elapsed{0};
  uint64_t _start{SDL_GetTicks()};
};
}
