#pragma once

#include "common.hpp"

namespace geometry {
class margin final {
public:
  constexpr margin() noexcept = default;
  constexpr margin(const margin&) noexcept = default;
  constexpr margin(margin&&) noexcept = default;

  constexpr margin(float top, float left, float bottom, float right) noexcept
    : _top(top), _left(left), _bottom(bottom), _right(right) {}

  constexpr ~margin() noexcept = default;

  constexpr margin& operator=(const margin&) noexcept = default;
  constexpr margin& operator=(margin&&) noexcept = default;

  [[nodiscard]] constexpr float top() const noexcept { return _top; }
  [[nodiscard]] constexpr float left() const noexcept { return _left; }
  [[nodiscard]] constexpr float bottom() const noexcept { return _bottom; }
  [[nodiscard]] constexpr float right() const noexcept { return _right; }

  constexpr void set_top(numeric auto value) noexcept {
    _top = static_cast<float>(value);
  }

  constexpr void set_left(numeric auto value) noexcept {
    _left = static_cast<float>(value);
  }

  constexpr void set_bottom(numeric auto value) noexcept {
    _bottom = static_cast<float>(value);
  }

  constexpr void set_right(numeric auto value) noexcept {
    _right = static_cast<float>(value);
  }

  constexpr void set(numeric auto top, numeric auto left, numeric auto bottom, numeric auto right) noexcept {
    _top = static_cast<float>(top);
    _left = static_cast<float>(left);
    _bottom = static_cast<float>(bottom);
    _right = static_cast<float>(right);
  }

  [[nodiscard]] constexpr float operator[](std::size_t index) const noexcept {
    [[assume(index < 4)]];
    switch (index) {
      case 0: return _top;
      case 1: return _left;
      case 2: return _bottom;
      case 3: return _right;
      default: std::unreachable();
    }
  }

  constexpr float& operator[](std::size_t index) noexcept {
    [[assume(index < 4)]];
    switch (index) {
      case 0: return _top;
      case 1: return _left;
      case 2: return _bottom;
      case 3: return _right;
      default: std::unreachable();
    }
  }

  [[nodiscard]] constexpr margin operator*(numeric auto factor) const noexcept {
    return margin(
      _top * static_cast<float>(factor),
      _left * static_cast<float>(factor),
      _bottom * static_cast<float>(factor),
      _right * static_cast<float>(factor)
    );
  }

  constexpr margin& operator*=(numeric auto factor) noexcept {
    _top *= static_cast<float>(factor);
    _left *= static_cast<float>(factor);
    _bottom *= static_cast<float>(factor);
    _right *= static_cast<float>(factor);
    return *this;
  }

  [[nodiscard]] constexpr margin operator/(numeric auto factor) const noexcept {
    return margin(
      _top / static_cast<float>(factor),
      _left / static_cast<float>(factor),
      _bottom / static_cast<float>(factor),
      _right / static_cast<float>(factor)
    );
  }

  constexpr margin& operator/=(numeric auto factor) noexcept {
    _top /= static_cast<float>(factor);
    _left /= static_cast<float>(factor);
    _bottom /= static_cast<float>(factor);
    _right /= static_cast<float>(factor);
    return *this;
  }

  [[nodiscard]] constexpr margin operator+(const margin& other) const noexcept {
    return margin(
      _top + other._top,
      _left + other._left,
      _bottom + other._bottom,
      _right + other._right
    );
  }

  constexpr margin& operator+=(const margin& other) noexcept {
    _top += other._top;
    _left += other._left;
    _bottom += other._bottom;
    _right += other._right;
    return *this;
  }

  [[nodiscard]] constexpr bool operator==(const margin&) const noexcept = default;

  template<std::size_t I>
  [[nodiscard]] constexpr decltype(auto) get() const noexcept {
    [[assume(I < 4)]];
    if constexpr (I == 0) return top();
    else if constexpr (I == 1) return left();
    else if constexpr (I == 2) return bottom();
    else if constexpr (I == 3) return right();
  }

  template<std::size_t I>
  [[nodiscard]] constexpr decltype(auto) get() noexcept {
    [[assume(I < 4)]];
    if constexpr (I == 0) return (_top);
    else if constexpr (I == 1) return (_left);
    else if constexpr (I == 2) return (_bottom);
    else if constexpr (I == 3) return (_right);
  }

  friend void from_json(const nlohmann::json& j, margin& m);

private:
  float _top{.0f};
  float _left{.0f};
  float _bottom{.0f};
  float _right{.0f};
};

[[nodiscard]] inline constexpr margin operator*(numeric auto factor, const margin& m) noexcept {
  return m * factor;
}
}

template<>
struct std::tuple_size<geometry::margin> : std::integral_constant<std::size_t, 4> {};

template<std::size_t I>
struct std::tuple_element<I, geometry::margin> {
  using type = float;
};


