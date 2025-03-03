#include "color.hpp"

using namespace graphics;

color::color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
    : _r(r), _g(g), _b(b), _a(a) {}

color::color(const SDL_Color &scolor) noexcept
    : color(scolor.r, scolor.g, scolor.b, scolor.a) {}

color::color(const std::string &hex)
    : _r(0), _g(0), _b(0), _a(255) {
  if (hex.length() != 7 && hex.length() != 9) [[unlikely]] {
    throw std::invalid_argument(fmt::format("Invalid hex code format: '{}'. Use #RRGGBB or #RRGGBBAA.", hex));
  }

  if (hex[0] != '#') [[unlikely]] {
    throw std::invalid_argument(fmt::format("Hex code '{}' must start with '#'.", hex));
  }

  _r = static_cast<uint8_t>(std::stoi(hex.substr(1, 2), nullptr, 16));
  _g = static_cast<uint8_t>(std::stoi(hex.substr(3, 2), nullptr, 16));
  _b = static_cast<uint8_t>(std::stoi(hex.substr(5, 2), nullptr, 16));
  if (hex.length() == 9) {
    _a = static_cast<uint8_t>(std::stoi(hex.substr(7, 2), nullptr, 16));
  }
}

color::color(uint32_t pixel, const SDL_PixelFormat *format) noexcept {
  SDL_GetRGBA(pixel, format, &_r, &_g, &_b, &_a);
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

void color::set_r(uint8_t r) noexcept {
  _r = r;
}
void color::set_g(uint8_t g) noexcept {
  _g = g;
}
void color::set_b(uint8_t b) noexcept {
  _b = b;
}
void color::set_a(uint8_t a) noexcept {
  _a = a;
}

bool color::operator==(const color &other) const noexcept {
  return std::tie(_r, _g, _b, _a) == std::tie(other._r, other._g, other._b, other._a);
}

bool color::operator!=(const color &other) const noexcept {
  return !(*this == other);
}

color::operator SDL_Color() const noexcept {
  return SDL_Color{_r, _g, _b, _a};
}
