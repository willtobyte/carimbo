#pragma once

#include "common.hpp"

namespace audio {
class soundfx final {
public:
  explicit soundfx(const std::string& filename, bool retro = false);
  ~soundfx() noexcept;

  void play(bool loop = false) const noexcept;
  void stop() const noexcept;

  void update(float_t delta) noexcept;

  void set_onbegin(std::function<void()>&& callback) noexcept;
  void set_onend(std::function<void()>&& callback) noexcept;

private:
  ALuint _source{0};
  ALuint _buffer{0};

  std::function<void()> _onbegin{};
  std::function<void()> _onend{};
  mutable std::atomic<bool> _notified{false};
};
}
