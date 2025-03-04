#pragma once

#include "common.hpp"

namespace graphics {
class color {
public:
  color() = default;
  color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept;
  color(const SDL_Color &color) noexcept;
  color(const std::string &hex);
  color(uint32_t pixel, SDL_PixelFormat *format) noexcept;
  ~color() = default;

  uint8_t r() const noexcept;
  uint8_t g() const noexcept;
  uint8_t b() const noexcept;
  uint8_t a() const noexcept;

  void set_r(uint8_t r) noexcept;
  void set_g(uint8_t g) noexcept;
  void set_b(uint8_t b) noexcept;
  void set_a(uint8_t a) noexcept;

  bool operator==(const color &other) const noexcept;

  bool operator!=(const color &other) const noexcept;

  explicit operator SDL_Color() const noexcept;

private:
  uint8_t _r{0};
  uint8_t _g{0};
  uint8_t _b{0};
  uint8_t _a{255};
};
}
