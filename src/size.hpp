#pragma once

#include "common.hpp"

namespace geometry {
class size final {
public:
  constexpr size() noexcept = default;
  constexpr size(const size&) noexcept = default;
  constexpr size(size&&) noexcept = default;

  constexpr size(float width, float height) noexcept
    : _width(width), _height(height) {}

  constexpr ~size() noexcept = default;

  constexpr size& operator=(const size&) noexcept = default;
  constexpr size& operator=(size&&) noexcept = default;

  constexpr void set_width(float width) noexcept { _width = width; }
  constexpr void set_height(float height) noexcept { _height = height; }

  [[nodiscard]] constexpr float width() const noexcept { return _width; }
  [[nodiscard]] constexpr float height() const noexcept { return _height; }

  [[nodiscard]] constexpr float operator[](std::size_t index) const noexcept {
    [[assume(index < 2)]];
    return index == 0 ? _width : _height;
  }

  constexpr float& operator[](std::size_t index) noexcept {
    [[assume(index < 2)]];
    return index == 0 ? _width : _height;
  }

  [[nodiscard]] constexpr size operator*(numeric auto factor) const noexcept {
    return size(
      _width * static_cast<float>(factor),
      _height * static_cast<float>(factor)
    );
  }

  constexpr size& operator*=(numeric auto factor) noexcept {
    _width *= static_cast<float>(factor);
    _height *= static_cast<float>(factor);
    return *this;
  }

  [[nodiscard]] constexpr size operator/(numeric auto factor) const noexcept {
    return size(
      _width / static_cast<float>(factor),
      _height / static_cast<float>(factor)
    );
  }

  constexpr size& operator/=(numeric auto factor) noexcept {
    _width /= static_cast<float>(factor);
    _height /= static_cast<float>(factor);
    return *this;
  }

  [[nodiscard]] constexpr size operator+(const size& other) const noexcept {
    return size(_width + other._width, _height + other._height);
  }

  constexpr size& operator+=(const size& other) noexcept {
    _width += other._width;
    _height += other._height;
    return *this;
  }

  [[nodiscard]] constexpr size operator-(const size& other) const noexcept {
    return size(_width - other._width, _height - other._height);
  }

  constexpr size& operator-=(const size& other) noexcept {
    _width -= other._width;
    _height -= other._height;
    return *this;
  }

  [[nodiscard]] CONSTEXPR_IF_NOT_MSVC bool operator==(const size& rhs) const noexcept {
    return std::abs(_width - rhs._width) <= epsilon * std::max(std::abs(_width), std::abs(rhs._width))
        && std::abs(_height - rhs._height) <= epsilon * std::max(std::abs(_height), std::abs(rhs._height));
  }

  [[nodiscard]] constexpr float area() const noexcept {
    return _width * _height;
  }

  [[nodiscard]] constexpr float aspect_ratio() const noexcept {
    return _height != 0.0f ? _width / _height : 0.0f;
  }

  [[nodiscard]] constexpr bool is_empty() const noexcept {
    return _width <= 0.0f || _height <= 0.0f;
  }

  template<std::size_t I>
  [[nodiscard]] constexpr decltype(auto) get() const noexcept {
    [[assume(I < 2)]];
    if constexpr (I == 0) return width();
    else if constexpr (I == 1) return height();
  }

  template<std::size_t I>
  [[nodiscard]] constexpr decltype(auto) get() noexcept {
    [[assume(I < 2)]];
    if constexpr (I == 0) return (_width);
    else if constexpr (I == 1) return (_height);
  }

  friend void from_json(const nlohmann::json& j, size& s);

private:
  float _width{0};
  float _height{0};
};

[[nodiscard]] inline constexpr size operator*(numeric auto factor, const size& s) noexcept {
  return s * factor;
}
}

template<>
struct std::tuple_size<geometry::size> : std::integral_constant<std::size_t, 2> {};

template<std::size_t I>
struct std::tuple_element<I, geometry::size> {
  using type = float;
};

template<>
struct std::formatter<geometry::size> : std::formatter<std::string> {
  auto format(const geometry::size& s, std::format_context& context) const {
    return std::formatter<std::string>::format(
      std::format("size({}x{})", s.width(), s.height()),
      context
    );
  }
};
