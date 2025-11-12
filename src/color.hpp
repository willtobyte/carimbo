#pragma once

#include "common.hpp"

namespace graphics {
class color final {
public:
  color() = default;
  explicit color(uint32_t pixel) noexcept;
  explicit color(const SDL_Color& scolor) noexcept;
  color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept;
  explicit color(std::string_view hex) noexcept;
  ~color() = default;

  uint8_t r() const noexcept;
  uint8_t g() const noexcept;
  uint8_t b() const noexcept;
  uint8_t a() const noexcept;

  void set_r(uint8_t r) noexcept;
  void set_g(uint8_t g) noexcept;
  void set_b(uint8_t b) noexcept;
  void set_a(uint8_t a) noexcept;

  bool operator==(const color& other) const noexcept;

  bool operator!=(const color& other) const noexcept;

  explicit operator SDL_Color() const noexcept;

private:
  uint8_t _r{0};
  uint8_t _g{0};
  uint8_t _b{0};
  uint8_t _a{255};
};
}
