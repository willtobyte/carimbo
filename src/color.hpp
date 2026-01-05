#pragma once

#include "common.hpp"

namespace {
static constexpr uint8_t _lookup[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
  0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

[[nodiscard]] static constexpr uint8_t from_hex(char c) noexcept {
  return _lookup[static_cast<uint8_t>(c)];
}
}

class color {
public:
  union {
    struct { uint8_t r, g, b, a; };
    std::array<uint8_t, 4> _data;
    uint32_t _value;
  };

  constexpr color() noexcept : r(0), g(0), b(0), a(255) {}

  constexpr color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept
      : r(r), g(g), b(b), a(a) {}

  constexpr explicit color(std::array<uint8_t, 4> const& data) noexcept
      : _data(data) {}

  explicit constexpr color(uint32_t pixel) noexcept
      : r(static_cast<uint8_t>(pixel >> 24)),
        g(static_cast<uint8_t>(pixel >> 16)),
        b(static_cast<uint8_t>(pixel >> 8)),
        a(static_cast<uint8_t>(pixel)) {}

  explicit constexpr color(SDL_Color const& scolor) noexcept
      : r(scolor.r), g(scolor.g), b(scolor.b), a(scolor.a) {}

  explicit constexpr color(std::string_view hex) noexcept : color() {
    const auto n = hex.size();
    const bool valid = (n == 7 || n == 9) && hex[0] == '#';

    if (!valid) [[unlikely]] {
      return;
    }

    r = static_cast<uint8_t>((from_hex(hex[1]) << 4) | from_hex(hex[2]));
    g = static_cast<uint8_t>((from_hex(hex[3]) << 4) | from_hex(hex[4]));
    b = static_cast<uint8_t>((from_hex(hex[5]) << 4) | from_hex(hex[6]));
    a = (n == 9) ? static_cast<uint8_t>((from_hex(hex[7]) << 4) | from_hex(hex[8])) : 255;
  }

  [[nodiscard]] constexpr uint8_t operator[](std::size_t i) const noexcept {
    return _data[i];
  }

  [[nodiscard]] constexpr uint8_t& operator[](std::size_t i) noexcept {
    return _data[i];
  }

  [[nodiscard]] constexpr color with_alpha(uint8_t alpha) const noexcept {
    return color(r, g, b, alpha);
  }

  [[nodiscard]] constexpr color lerp(color const& other, float t) const noexcept {
    const float inv_t = 1.0f - t;
    return color(
      static_cast<uint8_t>(r * inv_t + other.r * t),
      static_cast<uint8_t>(g * inv_t + other.g * t),
      static_cast<uint8_t>(b * inv_t + other.b * t),
      static_cast<uint8_t>(a * inv_t + other.a * t)
    );
  }

  [[nodiscard]] constexpr bool operator==(color const& other) const noexcept {
    return _value == other._value;
  }

  [[nodiscard]] constexpr bool operator!=(color const& other) const noexcept {
    return _value != other._value;
  }

  [[nodiscard]] operator SDL_Color() const noexcept {
    return SDL_Color{ r, g, b, a };
  }
};
