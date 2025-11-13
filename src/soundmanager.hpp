#pragma once

#include "common.hpp"

namespace audio {
class soundmanager final {
public:
  soundmanager(std::shared_ptr<audiodevice> audiodevice);
  ~soundmanager() = default;

  std::shared_ptr<soundfx> get(std::string_view filename);

  void play(std::string_view filename, bool loop = false);
  void stop(std::string_view filename);

  void flush();

  void update(float delta);

#ifndef NDEBUG
  void debug() const;
#endif

private:
  std::shared_ptr<audiodevice> _audiodevice;

  std::unordered_map<std::string, std::shared_ptr<soundfx>, string_hash, string_equal> _pool;
};
}
