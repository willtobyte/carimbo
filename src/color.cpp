#include "color.hpp"

using namespace graphics;

color::color(uint32_t pixel) {
  _r = static_cast<uint8_t>((pixel >> 24) & 0xFF);
  _g = static_cast<uint8_t>((pixel >> 16) & 0xFF);
  _b = static_cast<uint8_t>((pixel >> 8)  & 0xFF);
  _a = static_cast<uint8_t>(pixel & 0xFF);
}

color::color(const SDL_Color &scolor)
    : color(scolor.r, scolor.g, scolor.b, scolor.a) {}

color::color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    : _r(r), _g(g), _b(b), _a(a) {}

color::color(const std::string &hex)
    : _r(0), _g(0), _b(0), _a(255) {
  if (hex.length() != 7 && hex.length() != 9) [[unlikely]] {
    throw std::runtime_error(fmt::format("Invalid hex code format: '{}'. Use #RRGGBB or #RRGGBBAA.", hex));
  }

  if (hex[0] != '#') [[unlikely]] {
    throw std::runtime_error(fmt::format("Hex code '{}' must start with '#'.", hex));
  }

  _r = static_cast<uint8_t>(std::stoi(hex.substr(1, 2), nullptr, 16));
  _g = static_cast<uint8_t>(std::stoi(hex.substr(3, 2), nullptr, 16));
  _b = static_cast<uint8_t>(std::stoi(hex.substr(5, 2), nullptr, 16));
  if (hex.length() == 9) {
    _a = static_cast<uint8_t>(std::stoi(hex.substr(7, 2), nullptr, 16));
  }
}

uint8_t color::r() const {
  return _r;
}

uint8_t color::g() const {
  return _g;
}

uint8_t color::b() const {
  return _b;
}

uint8_t color::a() const {
  return _a;
}

void color::set_r(uint8_t r) {
  _r = r;
}

void color::set_g(uint8_t g) {
  _g = g;
}

void color::set_b(uint8_t b) {
  _b = b;
}

void color::set_a(uint8_t a) {
  _a = a;
}

bool color::operator==(const color &other) const {
  return std::tie(_r, _g, _b, _a) == std::tie(other._r, other._g, other._b, other._a);
}

bool color::operator!=(const color &other) const {
  return !(*this == other);
}

color::operator SDL_Color() const {
  return SDL_Color{_r, _g, _b, _a};
}
