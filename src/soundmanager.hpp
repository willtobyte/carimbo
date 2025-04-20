#pragma once

#include "common.hpp"
#include "soundfx.hpp"

namespace audio {
class soundmanager final {
public:
  soundmanager(std::shared_ptr<audiodevice> audiodevice);
  ~soundmanager() = default;

  std::shared_ptr<soundfx> get(const std::string &filename);
  void play(const std::string &filename, bool loop = false);
  void stop(const std::string &filename);
  void flush();

private:
  std::shared_ptr<audiodevice> _audiodevice;
  std::unordered_map<std::string, std::shared_ptr<soundfx>> _pool;
};
}
