#pragma once

#include "common.hpp"

namespace audio {
class soundfx {
public:
  soundfx(const std::string &filename);
  ~soundfx() noexcept;

  void play() const noexcept;
  void stop() const noexcept;

private:
  ALuint _source;
};
}
