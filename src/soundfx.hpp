#pragma once

#include "common.hpp"

namespace audio {
class soundfx final {
public:
  explicit soundfx(const std::string& filename);
  ~soundfx();

  void play(bool loop = false) const;
  void stop() const;

  void update(float delta);

  void set_volume(float gain);
  float volume() const;
    
  void set_onbegin(sol::protected_function callback);
  void set_onend(sol::protected_function callback);

private:
  ALuint _source{0};
  ALuint _buffer{0};

  std::function<void()> _onbegin{};
  std::function<void()> _onend{};
  mutable std::atomic<bool> _notified{false};
};
}
