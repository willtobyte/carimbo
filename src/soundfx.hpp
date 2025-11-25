#pragma once

#include "common.hpp"

class soundfx final {
public:
  explicit soundfx(std::string_view filename);
  ~soundfx();

  void play(bool loop = false) const noexcept;
  void stop() const noexcept;

  void update(float delta);

  void set_volume(float gain) noexcept;
  float volume() const noexcept;

  void set_onbegin(sol::protected_function callback);
  void set_onend(sol::protected_function callback);

private:
  ALuint _source{0};
  ALuint _buffer{0};

  std::function<void()> _onbegin{};
  std::function<void()> _onend{};
  mutable std::atomic<bool> _notified{false};
};