#pragma once

#include "common.hpp"

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

  boost::unordered_flat_map<std::string, std::shared_ptr<soundfx>, transparent_string_hash, std::equal_to<>> _pool;
};
