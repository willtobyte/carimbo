#pragma once

#include "common.hpp"

#include "point.hpp"
#include "size.hpp"

namespace geometry {

class rectangle final {
public:
  constexpr rectangle() noexcept = default;
  constexpr rectangle(const rectangle&) noexcept = default;
  constexpr rectangle(rectangle&&) noexcept = default;

  constexpr rectangle(float x, float y, float width, float height) noexcept
    : _position{ x, y }, _size{ width, height } {}

  constexpr rectangle(const geometry::point& position, const geometry::size& size) noexcept
    : _position(position), _size(size) {}

  constexpr ~rectangle() noexcept = default;

  constexpr rectangle& operator=(const rectangle&) noexcept = default;
  constexpr rectangle& operator=(rectangle&&) noexcept = default;

  [[nodiscard]] constexpr float x() const noexcept { return _position.x(); }
  [[nodiscard]] constexpr float y() const noexcept { return _position.y(); }
  [[nodiscard]] constexpr float width() const noexcept { return _size.width(); }
  [[nodiscard]] constexpr float height() const noexcept { return _size.height(); }

  [[nodiscard]] constexpr float operator[](std::size_t index) const noexcept {
    [[assume(index < 4)]];
    switch (index) {
      case 0: return x();
      case 1: return y();
      case 2: return width();
      case 3: return height();
      default: std::unreachable();
    }
  }

  constexpr void set_position(float x, float y) noexcept {
    _position = { x, y };
  }

  constexpr void set_position(const geometry::point& position) noexcept {
    _position = position;
  }

  [[nodiscard]] constexpr geometry::point position() const noexcept {
    return _position;
  }

  [[nodiscard]] constexpr geometry::size size() const noexcept {
    return _size;
  }

  constexpr void scale(numeric auto factor) noexcept {
    _size.set_width(_size.width() * static_cast<float>(factor));
    _size.set_height(_size.height() * static_cast<float>(factor));
  }

  [[nodiscard]] operator SDL_FRect() const noexcept {
    return SDL_FRect{
      .x = _position.x(),
      .y = _position.y(),
      .w = _size.width(),
      .h = _size.height()
    };
  }

  [[nodiscard]] constexpr rectangle operator+(const geometry::point& offset) const noexcept {
    return rectangle(_position + offset, _size);
  }

  [[nodiscard]] constexpr rectangle operator*(numeric auto factor) const noexcept {
    return rectangle(
      _position,
      geometry::size(
        _size.width() * static_cast<float>(factor),
        _size.height() * static_cast<float>(factor)
      )
    );
  }

  constexpr rectangle& operator*=(numeric auto factor) noexcept {
    scale(factor);
    return *this;
  }

  constexpr rectangle& operator+=(const geometry::point& offset) noexcept {
    _position = _position + offset;
    return *this;
  }

  [[nodiscard]] constexpr bool operator==(const rectangle&) const noexcept = default;

  template<std::size_t I>
  [[nodiscard]] constexpr decltype(auto) get() const noexcept {
    [[assume(I < 4)]];
    if constexpr (I == 0) return x();
    else if constexpr (I == 1) return y();
    else if constexpr (I == 2) return width();
    else if constexpr (I == 3) return height();
  }

  friend void from_json(const nlohmann::json& j, rectangle& r);

private:
  point _position{};
  geometry::size _size{};
};
}

template<>
struct std::tuple_size<geometry::rectangle> : std::integral_constant<std::size_t, 4> {};

template<std::size_t I>
struct std::tuple_element<I, geometry::rectangle> {
  using type = float;
};


