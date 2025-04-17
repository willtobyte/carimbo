#pragma once

#include "common.hpp"

namespace audio {
class soundfx {
public:
  soundfx(const std::string &filename);
  ~soundfx() noexcept;

  void play(bool loop) const noexcept;
  void stop() const noexcept;

private:
  ALuint _source;
};
}
