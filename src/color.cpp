#include "color.hpp"

using namespace graphics;

static constexpr uint8_t from_hex(char c) noexcept {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0;
}

color::color(const uint32_t pixel) noexcept
    : _r(static_cast<uint8_t>(pixel >> 24)),
      _g(static_cast<uint8_t>(pixel >> 16)),
      _b(static_cast<uint8_t>(pixel >> 8)),
      _a(static_cast<uint8_t>(pixel)) {}

color::color(const SDL_Color& scolor) noexcept
  : color(scolor.r, scolor.g, scolor.b, scolor.a) {}

color::color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) noexcept
    : _r(r), _g(g), _b(b), _a(a) {}

color::color(std::string_view hex) noexcept
    : _r(0), _g(0), _b(0), _a(255) {
  const auto n = hex.size();
  if (n != 7 && n != 9) [[unlikely]] {
    return;
  }

  if (hex[0] != '#') [[unlikely]] {
    return;
  }

  _r = (from_hex(hex[1]) << 4) | from_hex(hex[2]);
  _g = (from_hex(hex[3]) << 4) | from_hex(hex[4]);
  _b = (from_hex(hex[5]) << 4) | from_hex(hex[6]);

  if (n == 9) {
    _a = (from_hex(hex[7]) << 4) | from_hex(hex[8]);
  }
}

uint8_t color::r() const noexcept {
  return _r;
}

uint8_t color::g() const noexcept {
  return _g;
}

uint8_t color::b() const noexcept {
  return _b;
}

uint8_t color::a() const noexcept {
  return _a;
}

void color::set_r(const uint8_t r) noexcept {
  _r = r;
}

void color::set_g(const uint8_t g) noexcept {
  _g = g;
}

void color::set_b(const uint8_t b) noexcept {
  _b = b;
}

void color::set_a(const uint8_t a) noexcept {
  _a = a;
}

bool color::operator==(const color& other) const noexcept {
  return _r == other._r && _g == other._g && _b == other._b && _a == other._a;
}

bool color::operator!=(const color& other) const noexcept {
  return !(*this == other);
}

color::operator SDL_Color() const noexcept {
  return SDL_Color{_r, _g, _b, _a};
}
