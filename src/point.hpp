#pragma once

#include "common.hpp"

#include "size.hpp"

namespace geometry {
class point final {
public:
  constexpr point() noexcept = default;
  constexpr point(const point&) noexcept = default;
  constexpr point(point&&) noexcept = default;

  constexpr point(float x, float y) noexcept
    : _x(x), _y(y) {}

  constexpr ~point() noexcept = default;

  constexpr point& operator=(const point&) noexcept = default;
  constexpr point& operator=(point&&) noexcept = default;

  constexpr void set(float x, float y) noexcept {
    _x = x;
    _y = y;
  }

  constexpr void set_x(float x) noexcept { _x = x; }
  constexpr void set_y(float y) noexcept { _y = y; }

  [[nodiscard]] constexpr float x() const noexcept { return _x; }
  [[nodiscard]] constexpr float y() const noexcept { return _y; }

  [[nodiscard]] constexpr float operator[](std::size_t index) const noexcept {
    [[assume(index < 2)]];
    return index == 0 ? _x : _y;
  }

  constexpr float& operator[](std::size_t index) noexcept {
    [[assume(index < 2)]];
    return index == 0 ? _x : _y;
  }

  [[nodiscard]] operator SDL_FPoint() const noexcept {
    return SDL_FPoint{ _x, _y };
  }

  [[nodiscard]] constexpr point operator+(const point& other) const noexcept {
    return point(_x + other._x, _y + other._y);
  }

  constexpr point& operator+=(const point& other) noexcept {
    _x += other._x;
    _y += other._y;
    return *this;
  }

  constexpr point& operator+=(const std::pair<char, float>& o) noexcept {
    const auto [axis, value] = o;
    switch (axis) {
      case 'x': _x += value; break;
      case 'y': _y += value; break;
      default: std::unreachable();
    }

    return *this;
  }

  [[nodiscard]] constexpr point operator-(const size& rhs) const noexcept {
    return point(_x - rhs.width(), _y - rhs.height());
  }

  [[nodiscard]] constexpr point operator-(const point& rhs) const noexcept {
    return point(_x - rhs._x, _y - rhs._y);
  }

  constexpr point& operator-=(const point& other) noexcept {
    _x -= other._x;
    _y -= other._y;
    return *this;
  }

  [[nodiscard]] constexpr point operator*(numeric auto factor) const noexcept {
    return point(
      std::round(_x * static_cast<float>(factor)),
      std::round(_y * static_cast<float>(factor))
    );
  }

  constexpr point& operator*=(numeric auto factor) noexcept {
    _x = std::round(_x * static_cast<float>(factor));
    _y = std::round(_y * static_cast<float>(factor));
    return *this;
  }

  [[nodiscard]] constexpr point operator/(numeric auto factor) const noexcept {
    return point(_x / static_cast<float>(factor), _y / static_cast<float>(factor));
  }

  constexpr point& operator/=(numeric auto factor) noexcept {
    _x /= static_cast<float>(factor);
    _y /= static_cast<float>(factor);
    return *this;
  }

  [[nodiscard]] constexpr bool operator==(const point& other) const noexcept {
    return std::fabs(_x - other._x) <= epsilon * std::max(std::fabs(_x), std::fabs(other._x))
        && std::fabs(_y - other._y) <= epsilon * std::max(std::fabs(_y), std::fabs(other._y));
  }

  template<std::size_t I>
  [[nodiscard]] constexpr decltype(auto) get() const noexcept {
    [[assume(I < 2)]];
    if constexpr (I == 0) return x();
    else if constexpr (I == 1) return y();
  }

  template<std::size_t I>
  [[nodiscard]] constexpr decltype(auto) get() noexcept {
    [[assume(I < 2)]];
    if constexpr (I == 0) return (_x);
    else if constexpr (I == 1) return (_y);
  }

  friend void from_json(const nlohmann::json& j, point& p);

private:
  float _x{0};
  float _y{0};
};

[[nodiscard]] inline constexpr point operator*(const point& p, float factor) noexcept {
  return point(
    std::round(p.x() * factor),
    std::round(p.y() * factor)
  );
}

[[nodiscard]] inline constexpr point operator*(float factor, const point& p) noexcept {
  return p * factor;
}
}

template<>
struct std::tuple_size<geometry::point> : std::integral_constant<std::size_t, 2> {};

template<std::size_t I>
struct std::tuple_element<I, geometry::point> {
  using type = float;
};

template<>
struct std::formatter<geometry::point> : std::formatter<std::string> {
  auto format(const geometry::point& p, std::format_context& ctx) const {
    return std::formatter<std::string>::format(
      std::format("point({}, {})", p.x(), p.y()),
      ctx
    );
  }
};
