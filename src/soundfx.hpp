#pragma once

#include "common.hpp"

namespace audio {
class soundfx final {
public:
  soundfx(const std::string &filename);
  ~soundfx();

  void play(bool loop) const;
  void stop() const;

private:
  ALuint _source;
};
}
