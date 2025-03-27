#pragma once

#include "common.hpp"

namespace graphics {
class color {
public:
  constexpr color() = default;
  color(const std::string &hex);
  color(uint32_t pixel, const SDL_PixelFormat *format) noexcept;
  constexpr color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
      : _r(r), _g(g), _b(b), _a(a) {}
  constexpr color(const SDL_Color &scolor) noexcept
      : color(scolor.r, scolor.g, scolor.b, scolor.a) {}

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
