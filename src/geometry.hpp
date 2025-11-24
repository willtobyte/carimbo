#pragma once

#include "common.hpp"

struct alignas(8) vec2 {
  using value_type = float;

  union {
    struct { float x, y; };
    std::array<float, 2> _data;
  };

  constexpr vec2() noexcept : x(0), y(0) {}

  constexpr vec2(float x, float y) noexcept
      : x(x), y(y) {}

  constexpr explicit vec2(std::array<float, 2> const& data) noexcept
      : _data(data) {}

  [[nodiscard]] constexpr float operator[](std::size_t i) const noexcept {
    return _data[i];
  }

  [[nodiscard]] constexpr float& operator[](std::size_t i) noexcept {
    return _data[i];
  }

  [[nodiscard]] constexpr float const* data() const noexcept { return _data.data(); }
  [[nodiscard]] constexpr float* data() noexcept { return _data.data(); }

  [[nodiscard]] constexpr auto begin() noexcept { return _data.begin(); }
  [[nodiscard]] constexpr auto end() noexcept { return _data.end(); }
  [[nodiscard]] constexpr auto begin() const noexcept { return _data.begin(); }
  [[nodiscard]] constexpr auto end() const noexcept { return _data.end(); }

  [[nodiscard]] static constexpr std::size_t size() noexcept { return 2; }
  [[nodiscard]] static constexpr std::size_t max_size() noexcept { return 2; }
};

static_assert(sizeof(vec2) == 8);
static_assert(alignof(vec2) == 8);
static_assert(std::is_trivially_copyable_v<vec2>);
static_assert(std::is_standard_layout_v<vec2>);

struct alignas(16) box2 {
  using value_type = float;

  union {
    struct { float x, y, w, h; };
    std::array<float, 4> _data;
  };

  constexpr box2() noexcept : x(0), y(0), w(0), h(0) {}

  constexpr box2(float x, float y, float w, float h) noexcept
      : x(x), y(y), w(w), h(h) {}

  constexpr explicit box2(std::array<float, 4> const& data) noexcept
      : _data(data) {}

  constexpr box2(vec2 const& position, vec2 const& size) noexcept
      : x(position.x), y(position.y), w(size.x), h(size.y) {}

  [[nodiscard]] constexpr float operator[](std::size_t i) const noexcept {
    return _data[i];
  }

  [[nodiscard]] constexpr float& operator[](std::size_t i) noexcept {
    return _data[i];
  }

  [[nodiscard]] constexpr float const* data() const noexcept { return _data.data(); }
  [[nodiscard]] constexpr float* data() noexcept { return _data.data(); }

  [[nodiscard]] constexpr auto begin() noexcept { return _data.begin(); }
  [[nodiscard]] constexpr auto end() noexcept { return _data.end(); }
  [[nodiscard]] constexpr auto begin() const noexcept { return _data.begin(); }
  [[nodiscard]] constexpr auto end() const noexcept { return _data.end(); }

  [[nodiscard]] static constexpr std::size_t size() noexcept { return 4; }
  [[nodiscard]] static constexpr std::size_t max_size() noexcept { return 4; }

  [[nodiscard]] operator SDL_FRect() const noexcept {
    return SDL_FRect{
      .x = x,
      .y = y,
      .w = w,
      .h = h
    };
  }
};

static_assert(sizeof(box2) == 16);
static_assert(alignof(box2) == 16);
static_assert(std::is_trivially_copyable_v<box2>);
static_assert(std::is_standard_layout_v<box2>);

[[nodiscard]] constexpr auto operator+(vec2 const& lhs, vec2 const& rhs) noexcept {
  return vec2{lhs.x + rhs.x, lhs.y + rhs.y};
}

[[nodiscard]] constexpr auto operator+(box2 const& lhs, box2 const& rhs) noexcept {
  return box2{lhs.x + rhs.x, lhs.y + rhs.y, lhs.w + rhs.w, lhs.h + rhs.h};
}

[[nodiscard]] constexpr auto operator-(vec2 const& lhs, vec2 const& rhs) noexcept {
  return vec2{lhs.x - rhs.x, lhs.y - rhs.y};
}

[[nodiscard]] constexpr auto operator-(box2 const& lhs, box2 const& rhs) noexcept {
  return box2{lhs.x - rhs.x, lhs.y - rhs.y, lhs.w - rhs.w, lhs.h - rhs.h};
}

[[nodiscard]] constexpr auto operator*(vec2 const& lhs, float scalar) noexcept {
  return vec2{lhs.x * scalar, lhs.y * scalar};
}

[[nodiscard]] constexpr auto operator*(box2 const& lhs, float scalar) noexcept {
  return box2{lhs.x * scalar, lhs.y * scalar, lhs.w * scalar, lhs.h * scalar};
}

[[nodiscard]] constexpr auto operator*(float scalar, vec2 const& rhs) noexcept {
  return rhs * scalar;
}

[[nodiscard]] constexpr auto operator*(float scalar, box2 const& rhs) noexcept {
  return rhs * scalar;
}

[[nodiscard]] constexpr auto operator/(vec2 const& lhs, float scalar) noexcept {
  return vec2{lhs.x / scalar, lhs.y / scalar};
}

[[nodiscard]] constexpr auto operator/(box2 const& lhs, float scalar) noexcept {
  return box2{lhs.x / scalar, lhs.y / scalar, lhs.w / scalar, lhs.h / scalar};
}

constexpr auto& operator+=(vec2& lhs, vec2 const& rhs) noexcept {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}

constexpr auto& operator+=(box2& lhs, box2 const& rhs) noexcept {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  lhs.w += rhs.w;
  lhs.h += rhs.h;
  return lhs;
}

constexpr auto& operator-=(vec2& lhs, vec2 const& rhs) noexcept {
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  return lhs;
}

constexpr auto& operator-=(box2& lhs, box2 const& rhs) noexcept {
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  lhs.w -= rhs.w;
  lhs.h -= rhs.h;
  return lhs;
}

constexpr auto& operator*=(vec2& lhs, float scalar) noexcept {
  lhs.x *= scalar;
  lhs.y *= scalar;
  return lhs;
}

constexpr auto& operator*=(box2& lhs, float scalar) noexcept {
  lhs.x *= scalar;
  lhs.y *= scalar;
  lhs.w *= scalar;
  lhs.h *= scalar;
  return lhs;
}

constexpr auto& operator/=(vec2& lhs, float scalar) noexcept {
  lhs.x /= scalar;
  lhs.y /= scalar;
  return lhs;
}

constexpr auto& operator/=(box2& lhs, float scalar) noexcept {
  lhs.x /= scalar;
  lhs.y /= scalar;
  lhs.w /= scalar;
  lhs.h /= scalar;
  return lhs;
}

[[nodiscard]] constexpr bool operator==(vec2 const& lhs, vec2 const& rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

[[nodiscard]] constexpr bool operator==(box2 const& lhs, box2 const& rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.w == rhs.w && lhs.h == rhs.h;
}

[[nodiscard]] constexpr auto operator-(vec2 const& vec) noexcept {
  return vec2{-vec.x, -vec.y};
}

[[nodiscard]] constexpr auto operator-(box2 const& vec) noexcept {
  return box2{-vec.x, -vec.y, -vec.w, -vec.h};
}

[[nodiscard]] constexpr auto operator*(vec2 const& lhs, vec2 const& rhs) noexcept {
  return vec2{lhs.x * rhs.x, lhs.y * rhs.y};
}

[[nodiscard]] constexpr auto operator*(box2 const& lhs, box2 const& rhs) noexcept {
  return box2{lhs.x * rhs.x, lhs.y * rhs.y, lhs.w * rhs.w, lhs.h * rhs.h};
}

constexpr auto& operator*=(vec2& lhs, vec2 const& rhs) noexcept {
  lhs.x *= rhs.x;
  lhs.y *= rhs.y;
  return lhs;
}

constexpr auto& operator*=(box2& lhs, box2 const& rhs) noexcept {
  lhs.x *= rhs.x;
  lhs.y *= rhs.y;
  lhs.w *= rhs.w;
  lhs.h *= rhs.h;
  return lhs;
}

[[nodiscard]] constexpr float dot(vec2 const& lhs, vec2 const& rhs) noexcept {
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

[[nodiscard]] constexpr float dot(box2 const& lhs, box2 const& rhs) noexcept {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.w * rhs.w + lhs.h * rhs.h;
}

[[nodiscard]] constexpr float cross(vec2 const& lhs, vec2 const& rhs) noexcept {
  return lhs.x * rhs.y - lhs.y * rhs.x;
}

[[nodiscard]] constexpr float length_squared(vec2 const& vec) noexcept {
  return dot(vec, vec);
}

[[nodiscard]] constexpr float length_squared(box2 const& vec) noexcept {
  return dot(vec, vec);
}

[[nodiscard]] inline float length(vec2 const& vec) noexcept {
  return std::sqrt(length_squared(vec));
}

[[nodiscard]] inline float length(box2 const& vec) noexcept {
  return std::sqrt(length_squared(vec));
}

[[nodiscard]] inline float distance(vec2 const& a, vec2 const& b) noexcept {
  return length(b - a);
}

[[nodiscard]] inline float distance(box2 const& a, box2 const& b) noexcept {
  return length(b - a);
}

[[nodiscard]] inline float distance_squared(vec2 const& a, vec2 const& b) noexcept {
  return length_squared(b - a);
}

[[nodiscard]] inline float distance_squared(box2 const& a, box2 const& b) noexcept {
  return length_squared(b - a);
}

[[nodiscard]] inline auto normalize(vec2 const& vec) noexcept {
  const float len = length(vec);
  return len > 0.0f ? vec / len : vec2{};
}

[[nodiscard]] inline auto normalize(box2 const& vec) noexcept {
  const float len = length(vec);
  return len > 0.0f ? vec / len : box2{};
}

[[nodiscard]] constexpr auto lerp(vec2 const& a, vec2 const& b, float t) noexcept {
  return a + (b - a) * t;
}

[[nodiscard]] constexpr auto lerp(box2 const& a, box2 const& b, float t) noexcept {
  return a + (b - a) * t;
}

[[nodiscard]] constexpr auto clamp(vec2 const& vec, vec2 const& min, vec2 const& max) noexcept {
  return vec2{
    std::clamp(vec.x, min.x, max.x),
    std::clamp(vec.y, min.y, max.y)
  };
}

[[nodiscard]] constexpr auto clamp(box2 const& vec, box2 const& min, box2 const& max) noexcept {
  return box2{
    std::clamp(vec.x, min.x, max.x),
    std::clamp(vec.y, min.y, max.y),
    std::clamp(vec.w, min.w, max.w),
    std::clamp(vec.h, min.h, max.h)
  };
}

[[nodiscard]] constexpr auto min(vec2 const& a, vec2 const& b) noexcept {
  return vec2{
    std::min(a.x, b.x),
    std::min(a.y, b.y)
  };
}

[[nodiscard]] constexpr auto min(box2 const& a, box2 const& b) noexcept {
  return box2{
    std::min(a.x, b.x),
    std::min(a.y, b.y),
    std::min(a.w, b.w),
    std::min(a.h, b.h)
  };
}

[[nodiscard]] constexpr auto max(vec2 const& a, vec2 const& b) noexcept {
  return vec2{
    std::max(a.x, b.x),
    std::max(a.y, b.y)
  };
}

[[nodiscard]] constexpr auto max(box2 const& a, box2 const& b) noexcept {
  return box2{
    std::max(a.x, b.x),
    std::max(a.y, b.y),
    std::max(a.w, b.w),
    std::max(a.h, b.h)
  };
}

[[nodiscard]] inline float angle(vec2 const& vec) noexcept {
  return std::atan2(vec.y, vec.x);
}

[[nodiscard]] inline float angle_between(vec2 const& a, vec2 const& b) noexcept {
  const float d = dot(a, b);
  const float len_product = length(a) * length(b);
  return len_product > 0.0f ? std::acos(std::clamp(d / len_product, -1.0f, 1.0f)) : 0.0f;
}

[[nodiscard]] inline auto rotate(vec2 const& vec, float radians) noexcept {
  const float cos_a = std::cos(radians);
  const float sin_a = std::sin(radians);
  return vec2{
    vec.x * cos_a - vec.y * sin_a,
    vec.x * sin_a + vec.y * cos_a
  };
}

[[nodiscard]] constexpr auto perpendicular(vec2 const& vec) noexcept {
  return vec2{-vec.y, vec.x};
}

[[nodiscard]] inline auto reflect(vec2 const& incident, vec2 const& normal) noexcept {
  return incident - normal * (2.0f * dot(incident, normal));
}

[[nodiscard]] inline auto project(vec2 const& vec, vec2 const& onto) noexcept {
  const float len_sq = length_squared(onto);
  return len_sq > 0.0f ? onto * (dot(vec, onto) / len_sq) : vec2{};
}

inline void to_json(nlohmann::json& j, const vec2& v) {
  j = nlohmann::json{{"x", v.x}, {"y", v.y}};
}

inline void from_json(const nlohmann::json& j, vec2& v) {
  v.x = j.at("x").get<float>();
  v.y = j.at("y").get<float>();
}

inline void to_json(nlohmann::json& j, const box2& v) {
  j = nlohmann::json{{"x", v.x}, {"y", v.y}, {"width", v.w}, {"height", v.h}};
}

inline void from_json(const nlohmann::json& j, box2& v) {
  v.x = j.at("x").get<float>();
  v.y = j.at("y").get<float>();
  v.w = j.at("width").get<float>();
  v.h = j.at("height").get<float>();
}
