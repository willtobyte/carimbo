#pragma once

#include "common.hpp"

class soundfx final {
public:
  explicit soundfx(std::string_view filename);
  ~soundfx();

  soundfx(const soundfx&) = delete;
  soundfx& operator=(const soundfx&) = delete;
  soundfx(soundfx&&) = delete;
  soundfx& operator=(soundfx&&) = delete;

  void play(bool loop);
  void stop() noexcept;

  void update(float delta);

  void set_volume(float gain) noexcept;
  float volume() const noexcept;

  void set_onbegin(sol::protected_function callback);
  void set_onend(sol::protected_function callback);

private:
  std::vector<float> _samples;
  ma_audio_buffer _buffer{};
  ma_sound _sound{};

  functor _onbegin;
  functor _onend;
  std::atomic<bool> _ended{false};
};
