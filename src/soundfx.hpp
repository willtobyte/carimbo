#pragma once

#include "common.hpp"

class soundfx final {
public:
  explicit soundfx(std::string_view filename);
  ~soundfx() noexcept;

  void play(bool loop = false) const noexcept;
  void stop() const noexcept;

  void update(float delta) noexcept;

  void set_volume(float gain) noexcept;
  float volume() const noexcept;

  void set_onbegin(sol::protected_function callback) noexcept;
  void set_onend(sol::protected_function callback) noexcept;

private:
  ALuint _source{0};
  ALuint _buffer{0};

  std::function<void()> _onbegin{};
  std::function<void()> _onend{};
  mutable std::atomic<bool> _notified{false};
};
