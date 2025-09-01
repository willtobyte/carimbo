#pragma once

#include "common.hpp"

namespace audio {
class soundfx final {
public:
  explicit soundfx(const std::string& name, bool retro = false);
  ~soundfx() noexcept;

  void play(bool loop = false) const noexcept;
  void stop() const noexcept;

private:
  ALuint _source;
  ALuint _buffer{0};
};
}
