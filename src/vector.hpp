#pragma once

#include "common.hpp"

namespace math {
template <std::size_t N>
class vector {
public:
  constexpr vector() noexcept = default;

  constexpr explicit vector(std::array<float, N> const& data) noexcept
      : _data(data) {}

  template <typename... Args>
      requires (sizeof...(Args) == N) && (std::same_as<Args, float> && ...)
  constexpr vector(Args... args) noexcept
      : _data{static_cast<float>(args)...} {}

  [[nodiscard]] constexpr float operator[](std::size_t i) const noexcept {
    return _data[i];
  }

  [[nodiscard]] constexpr float& operator[](std::size_t i) noexcept {
    return _data[i];
  }

private:
  std::array<float, N> _data{};
};

template <>
class vector<2> {
public:
  union {
    struct { float x, y; };
    std::array<float, 2> _data;
  };

  constexpr vector() noexcept : x(0), y(0) {}

  constexpr vector(float x, float y) noexcept
      : x(x), y(y) {}

  constexpr explicit vector(std::array<float, 2> const& data) noexcept
      : _data(data) {}

  [[nodiscard]] constexpr float operator[](std::size_t i) const noexcept {
    return _data[i];
  }

  [[nodiscard]] constexpr float& operator[](std::size_t i) noexcept {
    return _data[i];
  }
};

template <>
class vector<4> {
public:
  union {
    struct { float x, y, w, h; };
    std::array<float, 4> _data;
  };

  constexpr vector() noexcept : x(0), y(0), w(0), h(0) {}

  constexpr vector(float x, float y, float w, float h) noexcept
      : x(x), y(y), w(w), h(h) {}

  constexpr explicit vector(std::array<float, 4> const& data) noexcept
      : _data(data) {}

  [[nodiscard]] constexpr float operator[](std::size_t i) const noexcept {
    return _data[i];
  }

  [[nodiscard]] constexpr float& operator[](std::size_t i) noexcept {
    return _data[i];
  }

  [[nodiscard]] operator SDL_FRect() const noexcept {
    return SDL_FRect{
      .x = x,
      .y = y,
      .w = w,
      .h = h
    };
  }
};

template <std::size_t N>
[[nodiscard]] constexpr auto operator+(vector<N> const& lhs, vector<N> const& rhs) noexcept {
  vector<N> result;
  for (std::size_t i = 0; i < N; ++i) {
    result[i] = lhs[i] + rhs[i];
  }

  return result;
}

template <std::size_t N>
[[nodiscard]] constexpr auto operator-(vector<N> const& lhs, vector<N> const& rhs) noexcept {
  vector<N> result;
  for (std::size_t i = 0; i < N; ++i) {
    result[i] = lhs[i] - rhs[i];
  }

  return result;
}

template <std::size_t N>
[[nodiscard]] constexpr auto operator*(vector<N> const& lhs, float scalar) noexcept {
  vector<N> result;
  for (std::size_t i = 0; i < N; ++i) {
    result[i] = lhs[i] * scalar;
  }

  return result;
}

template <std::size_t N>
[[nodiscard]] constexpr auto operator*(float scalar, vector<N> const& rhs) noexcept {
  return rhs * scalar;
}

template <std::size_t N>
[[nodiscard]] constexpr auto operator/(vector<N> const& lhs, float scalar) noexcept {
  vector<N> result;
  for (std::size_t i = 0; i < N; ++i) {
    result[i] = lhs[i] / scalar;
  }

  return result;
}

template <std::size_t N>
constexpr auto& operator+=(vector<N>& lhs, vector<N> const& rhs) noexcept {
  for (std::size_t i = 0; i < N; ++i) {
    lhs[i] += rhs[i];
  }

  return lhs;
}

template <std::size_t N>
constexpr auto& operator-=(vector<N>& lhs, vector<N> const& rhs) noexcept {
  for (std::size_t i = 0; i < N; ++i) {
    lhs[i] -= rhs[i];
  }

  return lhs;
}

template <std::size_t N>
constexpr auto& operator*=(vector<N>& lhs, float scalar) noexcept {
  for (std::size_t i = 0; i < N; ++i) {
    lhs[i] *= scalar;
  }

  return lhs;
}

template <std::size_t N>
constexpr auto& operator/=(vector<N>& lhs, float scalar) noexcept {
  for (std::size_t i = 0; i < N; ++i) {
    lhs[i] /= scalar;
  }

  return lhs;
}

template <std::size_t N>
[[nodiscard]] constexpr bool operator==(vector<N> const& lhs, vector<N> const& rhs) noexcept {
  for (std::size_t i = 0; i < N; ++i) {
    if (lhs[i] != rhs[i]) return false;
  }

  return true;
}

template <std::size_t N>
[[nodiscard]] constexpr auto operator-(vector<N> const& vec) noexcept {
  vector<N> result;
  for (std::size_t i = 0; i < N; ++i) {
    result[i] = -vec[i];
  }

  return result;
}

template <std::size_t N>
[[nodiscard]] constexpr auto operator*(vector<N> const& lhs, vector<N> const& rhs) noexcept {
  vector<N> result;
  for (std::size_t i = 0; i < N; ++i) {
    result[i] = lhs[i] * rhs[i];
  }

  return result;
}

template <std::size_t N>
constexpr auto& operator*=(vector<N>& lhs, vector<N> const& rhs) noexcept {
  for (std::size_t i = 0; i < N; ++i) {
    lhs[i] *= rhs[i];
  }

  return lhs;
}

template <std::size_t N>
[[nodiscard]] constexpr float dot(vector<N> const& lhs, vector<N> const& rhs) noexcept {
  float result = 0.0f;
  for (std::size_t i = 0; i < N; ++i) {
    result += lhs[i] * rhs[i];
  }

  return result;
}

template <std::size_t N>
[[nodiscard]] constexpr float length_squared(vector<N> const& vec) noexcept {
  return dot(vec, vec);
}

template <std::size_t N>
[[nodiscard]] inline float length(vector<N> const& vec) noexcept {
  return std::sqrt(length_squared(vec));
}

using vec2 = vector<2>;
using vec4 = vector<4>;

template <std::size_t N>
[[nodiscard]] inline auto normalize(vector<N> const& vec) noexcept {
  return vec / length(vec);
}

inline void to_json(nlohmann::json& j, const vec2& v) {
  j = nlohmann::json{{"x", v.x}, {"y", v.y}};
}

inline void from_json(const nlohmann::json& j, vec2& v) {
  v.x = j.at("x").get<float>();
  v.y = j.at("y").get<float>();
}

inline void to_json(nlohmann::json& j, const vec4& v) {
  j = nlohmann::json{{"x", v.x}, {"y", v.y}, {"width", v.w}, {"height", v.h}};
}

inline void from_json(const nlohmann::json& j, vec4& v) {
  v.x = j.at("x").get<float>();
  v.y = j.at("y").get<float>();
  v.w = j.at("width").get<float>();
  v.h = j.at("height").get<float>();
}
}
