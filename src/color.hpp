#pragma once

#include "common.hpp"

namespace graphics {

static constexpr uint8_t lookup[256] = {
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
  return lookup[static_cast<uint8_t>(c)];
}

class color final {
public:
  constexpr color() noexcept = default;
  constexpr color(const color&) noexcept = default;
  constexpr color(color&&) noexcept = default;

  explicit constexpr color(uint32_t pixel) noexcept
    : _r(static_cast<uint8_t>(pixel >> 24)),
      _g(static_cast<uint8_t>(pixel >> 16)),
      _b(static_cast<uint8_t>(pixel >> 8)),
      _a(static_cast<uint8_t>(pixel)) {}

  explicit constexpr color(const SDL_Color& scolor) noexcept
    : _r(scolor.r), _g(scolor.g), _b(scolor.b), _a(scolor.a) {}

  constexpr color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept
    : _r(r), _g(g), _b(b), _a(a) {}

  explicit constexpr color(std::string_view hex) noexcept
    : _r(0), _g(0), _b(0), _a(255) {
    const auto n = hex.size();
    const bool valid = (n == 7 || n == 9) && hex[0] == '#';
    
    if (!valid) [[unlikely]] {
      return;
    }

    _r = (from_hex(hex[1]) << 4) | from_hex(hex[2]);
    _g = (from_hex(hex[3]) << 4) | from_hex(hex[4]);
    _b = (from_hex(hex[5]) << 4) | from_hex(hex[6]);
    _a = (n == 9) ? ((from_hex(hex[7]) << 4) | from_hex(hex[8])) : 255;
  }

  constexpr ~color() noexcept = default;

  constexpr color& operator=(const color&) noexcept = default;
  constexpr color& operator=(color&&) noexcept = default;

  [[nodiscard]] constexpr uint8_t r() const noexcept { return _r; }
  [[nodiscard]] constexpr uint8_t g() const noexcept { return _g; }
  [[nodiscard]] constexpr uint8_t b() const noexcept { return _b; }
  [[nodiscard]] constexpr uint8_t a() const noexcept { return _a; }

  constexpr void set_r(uint8_t r) noexcept { _r = r; }
  constexpr void set_g(uint8_t g) noexcept { _g = g; }
  constexpr void set_b(uint8_t b) noexcept { _b = b; }
  constexpr void set_a(uint8_t a) noexcept { _a = a; }

  [[nodiscard]] constexpr uint8_t operator[](std::size_t index) const noexcept {
    [[assume(index < 4)]];
    switch (index) {
      case 0: return _r;
      case 1: return _g;
      case 2: return _b;
      case 3: return _a;
      default: std::unreachable();
    }
  }

  constexpr uint8_t& operator[](std::size_t index) noexcept {
    [[assume(index < 4)]];
    switch (index) {
      case 0: return _r;
      case 1: return _g;
      case 2: return _b;
      case 3: return _a;
      default: std::unreachable();
    }
  }

  [[nodiscard]] constexpr uint32_t to_uint32() const noexcept {
    return (static_cast<uint32_t>(_r) << 24) |
           (static_cast<uint32_t>(_g) << 16) |
           (static_cast<uint32_t>(_b) << 8) |
           static_cast<uint32_t>(_a);
  }

  [[nodiscard]] constexpr color with_alpha(uint8_t alpha) const noexcept {
    return color(_r, _g, _b, alpha);
  }

  [[nodiscard]] constexpr color lerp(const color& other, float t) const noexcept {
    const float inv_t = 1.0f - t;
    return color(
      static_cast<uint8_t>(_r * inv_t + other._r * t),
      static_cast<uint8_t>(_g * inv_t + other._g * t),
      static_cast<uint8_t>(_b * inv_t + other._b * t),
      static_cast<uint8_t>(_a * inv_t + other._a * t)
    );
  }

  [[nodiscard]] constexpr bool operator==(const color&) const noexcept = default;

  [[nodiscard]] operator SDL_Color() const noexcept {
    return SDL_Color{ _r, _g, _b, _a };
  }

  template<std::size_t I>
  [[nodiscard]] constexpr decltype(auto) get() const noexcept {
    [[assume(I < 4)]];
    if constexpr (I == 0) return r();
    else if constexpr (I == 1) return g();
    else if constexpr (I == 2) return b();
    else if constexpr (I == 3) return a();
  }

  template<std::size_t I>
  [[nodiscard]] constexpr decltype(auto) get() noexcept {
    [[assume(I < 4)]];
    if constexpr (I == 0) return (_r);
    else if constexpr (I == 1) return (_g);
    else if constexpr (I == 2) return (_b);
    else if constexpr (I == 3) return (_a);
  }

private:
  uint8_t _r{0};
  uint8_t _g{0};
  uint8_t _b{0};
  uint8_t _a{255};
};

}

template<>
struct std::tuple_size<graphics::color> : std::integral_constant<std::size_t, 4> {};

template<std::size_t I>
struct std::tuple_element<I, graphics::color> {
  using type = uint8_t;
};

template<>
struct std::formatter<graphics::color> : std::formatter<std::string> {
  auto format(const graphics::color& c, std::format_context& ctx) const {
    return std::formatter<std::string>::format(
      std::format("color(#{:02x}{:02x}{:02x}{:02x})", c.r(), c.g(), c.b(), c.a()),
      ctx
    );
  }
};