#pragma once

#include "common.hpp"

namespace audio {
class soundfx {
public:
  soundfx(std::shared_ptr<audiodevice> audiodevice, const std::string &filename);
  ~soundfx() noexcept;

  void play() const noexcept;
  void stop() const noexcept;

private:
  std::shared_ptr<audiodevice> _audiodevice;
  ALuint _source;
};
}
