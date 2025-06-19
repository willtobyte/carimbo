#pragma once

#include "common.hpp"

namespace audio {
class soundfx final {
public:
  soundfx(const std::string& filename);
  ~soundfx() noexcept;

  void play(bool loop = false) const noexcept;
  void stop() const noexcept;

private:
  ALuint _source;
};
}
