#pragma once

#include "common.hpp"
#include "soundfx.hpp"

namespace audio {
class soundmanager final {
public:
  soundmanager(std::shared_ptr<audiodevice> audiodevice) noexcept;
  ~soundmanager() noexcept = default;

  std::shared_ptr<soundfx> get(const std::string& filename) noexcept;
  void play(const std::string& filename, bool loop = false) noexcept;
  void stop(const std::string& filename) noexcept;
  void flush() noexcept;

private:
  std::shared_ptr<audiodevice> _audiodevice;
  std::unordered_map<std::string, std::shared_ptr<soundfx>> _pool;
};
}
