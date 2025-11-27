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

   friend void from_json(const nlohmann::json& j, vec2& o);
};

static_assert(sizeof(vec2) == 8);
static_assert(alignof(vec2) == 8);
static_assert(std::is_trivially_copyable_v<vec2>);
static_assert(std::is_standard_layout_v<vec2>);

struct alignas(16) vec3 {
  using value_type = float;

  union {
    struct { float x, y, z; };
    std::array<float, 3> _data;
  };

  constexpr vec3() noexcept : x(0), y(0), z(0) {}

  constexpr vec3(float x, float y, float z) noexcept
      : x(x), y(y), z(z) {}

  constexpr explicit vec3(std::array<float, 3> const& data) noexcept
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

  [[nodiscard]] static constexpr std::size_t size() noexcept { return 3; }
  [[nodiscard]] static constexpr std::size_t max_size() noexcept { return 3; }

  friend void from_json(const nlohmann::json& j, vec3& o);
};

static_assert(sizeof(vec3) == 16);
static_assert(alignof(vec3) == 16);
static_assert(std::is_trivially_copyable_v<vec3>);
static_assert(std::is_standard_layout_v<vec3>);

struct alignas(16) quad {
  using value_type = float;

  union {
    struct { float x, y, w, h; };
    std::array<float, 4> _data;
  };

  constexpr quad() noexcept : x(0), y(0), w(0), h(0) {}

  constexpr quad(float x, float y, float w, float h) noexcept
      : x(x), y(y), w(w), h(h) {}

  constexpr explicit quad(std::array<float, 4> const& data) noexcept
      : _data(data) {}

  constexpr quad(vec2 const& position, vec2 const& size) noexcept
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

  friend void from_json(const nlohmann::json& j, quad& o);
};

static_assert(sizeof(quad) == 16);
static_assert(alignof(quad) == 16);
static_assert(std::is_trivially_copyable_v<quad>);
static_assert(std::is_standard_layout_v<quad>);

[[nodiscard]] constexpr auto operator+(vec2 const& lhs, vec2 const& rhs) noexcept {
  return vec2{lhs.x + rhs.x, lhs.y + rhs.y};
}

[[nodiscard]] constexpr auto operator+(vec3 const& lhs, vec3 const& rhs) noexcept {
  return vec3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

[[nodiscard]] constexpr auto operator+(quad const& lhs, quad const& rhs) noexcept {
  return quad{lhs.x + rhs.x, lhs.y + rhs.y, lhs.w + rhs.w, lhs.h + rhs.h};
}

[[nodiscard]] constexpr auto operator-(vec2 const& lhs, vec2 const& rhs) noexcept {
  return vec2{lhs.x - rhs.x, lhs.y - rhs.y};
}

[[nodiscard]] constexpr auto operator-(vec3 const& lhs, vec3 const& rhs) noexcept {
  return vec3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

[[nodiscard]] constexpr auto operator-(quad const& lhs, quad const& rhs) noexcept {
  return quad{lhs.x - rhs.x, lhs.y - rhs.y, lhs.w - rhs.w, lhs.h - rhs.h};
}

[[nodiscard]] constexpr auto operator*(vec2 const& lhs, float scalar) noexcept {
  return vec2{lhs.x * scalar, lhs.y * scalar};
}

[[nodiscard]] constexpr auto operator*(vec3 const& lhs, float scalar) noexcept {
  return vec3{lhs.x * scalar, lhs.y * scalar, lhs.z * scalar};
}

[[nodiscard]] constexpr auto operator*(quad const& lhs, float scalar) noexcept {
  return quad{lhs.x * scalar, lhs.y * scalar, lhs.w * scalar, lhs.h * scalar};
}

[[nodiscard]] constexpr auto operator*(float scalar, vec2 const& rhs) noexcept {
  return rhs * scalar;
}

[[nodiscard]] constexpr auto operator*(float scalar, vec3 const& rhs) noexcept {
  return rhs * scalar;
}

[[nodiscard]] constexpr auto operator*(float scalar, quad const& rhs) noexcept {
  return rhs * scalar;
}

[[nodiscard]] constexpr auto operator/(vec2 const& lhs, float scalar) noexcept {
  return vec2{lhs.x / scalar, lhs.y / scalar};
}

[[nodiscard]] constexpr auto operator/(vec3 const& lhs, float scalar) noexcept {
  return vec3{lhs.x / scalar, lhs.y / scalar, lhs.z / scalar};
}

[[nodiscard]] constexpr auto operator/(quad const& lhs, float scalar) noexcept {
  return quad{lhs.x / scalar, lhs.y / scalar, lhs.w / scalar, lhs.h / scalar};
}

constexpr auto& operator+=(vec2& lhs, vec2 const& rhs) noexcept {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}

constexpr auto& operator+=(vec3& lhs, vec3 const& rhs) noexcept {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  lhs.z += rhs.z;
  return lhs;
}

constexpr auto& operator+=(quad& lhs, quad const& rhs) noexcept {
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

constexpr auto& operator-=(vec3& lhs, vec3 const& rhs) noexcept {
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  lhs.z -= rhs.z;
  return lhs;
}

constexpr auto& operator-=(quad& lhs, quad const& rhs) noexcept {
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

constexpr auto& operator*=(vec3& lhs, float scalar) noexcept {
  lhs.x *= scalar;
  lhs.y *= scalar;
  lhs.z *= scalar;
  return lhs;
}

constexpr auto& operator*=(quad& lhs, float scalar) noexcept {
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

constexpr auto& operator/=(vec3& lhs, float scalar) noexcept {
  lhs.x /= scalar;
  lhs.y /= scalar;
  lhs.z /= scalar;
  return lhs;
}

constexpr auto& operator/=(quad& lhs, float scalar) noexcept {
  lhs.x /= scalar;
  lhs.y /= scalar;
  lhs.w /= scalar;
  lhs.h /= scalar;
  return lhs;
}

[[nodiscard]] constexpr bool operator==(vec2 const& lhs, vec2 const& rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

[[nodiscard]] constexpr bool operator==(vec3 const& lhs, vec3 const& rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

[[nodiscard]] constexpr bool operator==(quad const& lhs, quad const& rhs) noexcept {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.w == rhs.w && lhs.h == rhs.h;
}

[[nodiscard]] constexpr auto operator-(vec2 const& vec) noexcept {
  return vec2{-vec.x, -vec.y};
}

[[nodiscard]] constexpr auto operator-(vec3 const& vec) noexcept {
  return vec3{-vec.x, -vec.y, -vec.z};
}

[[nodiscard]] constexpr auto operator-(quad const& vec) noexcept {
  return quad{-vec.x, -vec.y, -vec.w, -vec.h};
}

[[nodiscard]] constexpr auto operator*(vec2 const& lhs, vec2 const& rhs) noexcept {
  return vec2{lhs.x * rhs.x, lhs.y * rhs.y};
}

[[nodiscard]] constexpr auto operator*(vec3 const& lhs, vec3 const& rhs) noexcept {
  return vec3{lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
}

[[nodiscard]] constexpr auto operator*(quad const& lhs, quad const& rhs) noexcept {
  return quad{lhs.x * rhs.x, lhs.y * rhs.y, lhs.w * rhs.w, lhs.h * rhs.h};
}

constexpr auto& operator*=(vec2& lhs, vec2 const& rhs) noexcept {
  lhs.x *= rhs.x;
  lhs.y *= rhs.y;
  return lhs;
}

constexpr auto& operator*=(vec3& lhs, vec3 const& rhs) noexcept {
  lhs.x *= rhs.x;
  lhs.y *= rhs.y;
  lhs.z *= rhs.z;
  return lhs;
}

constexpr auto& operator*=(quad& lhs, quad const& rhs) noexcept {
  lhs.x *= rhs.x;
  lhs.y *= rhs.y;
  lhs.w *= rhs.w;
  lhs.h *= rhs.h;
  return lhs;
}

[[nodiscard]] constexpr float dot(vec2 const& lhs, vec2 const& rhs) noexcept {
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

[[nodiscard]] constexpr float dot(vec3 const& lhs, vec3 const& rhs) noexcept {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

[[nodiscard]] constexpr float dot(quad const& lhs, quad const& rhs) noexcept {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.w * rhs.w + lhs.h * rhs.h;
}

[[nodiscard]] constexpr float cross(vec2 const& lhs, vec2 const& rhs) noexcept {
  return lhs.x * rhs.y - lhs.y * rhs.x;
}

[[nodiscard]] constexpr auto cross(vec3 const& lhs, vec3 const& rhs) noexcept {
  return vec3{
    lhs.y * rhs.z - lhs.z * rhs.y,
    lhs.z * rhs.x - lhs.x * rhs.z,
    lhs.x * rhs.y - lhs.y * rhs.x
  };
}

[[nodiscard]] constexpr float length_squared(vec2 const& vec) noexcept {
  return dot(vec, vec);
}

[[nodiscard]] constexpr float length_squared(vec3 const& vec) noexcept {
  return dot(vec, vec);
}

[[nodiscard]] constexpr float length_squared(quad const& vec) noexcept {
  return dot(vec, vec);
}

[[nodiscard]] inline float length(vec2 const& vec) noexcept {
  return std::sqrt(length_squared(vec));
}

[[nodiscard]] inline float length(vec3 const& vec) noexcept {
  return std::sqrt(length_squared(vec));
}

[[nodiscard]] inline float length(quad const& vec) noexcept {
  return std::sqrt(length_squared(vec));
}

[[nodiscard]] inline float distance(vec2 const& a, vec2 const& b) noexcept {
  return length(b - a);
}

[[nodiscard]] inline float distance(vec3 const& a, vec3 const& b) noexcept {
  return length(b - a);
}

[[nodiscard]] inline float distance(quad const& a, quad const& b) noexcept {
  return length(b - a);
}

[[nodiscard]] inline float distance_squared(vec2 const& a, vec2 const& b) noexcept {
  return length_squared(b - a);
}

[[nodiscard]] inline float distance_squared(vec3 const& a, vec3 const& b) noexcept {
  return length_squared(b - a);
}

[[nodiscard]] inline float distance_squared(quad const& a, quad const& b) noexcept {
  return length_squared(b - a);
}

[[nodiscard]] inline auto normalize(vec2 const& vec) noexcept {
  const float len = length(vec);
  return len > 0.0f ? vec / len : vec2{};
}

[[nodiscard]] inline auto normalize(vec3 const& vec) noexcept {
  const float len = length(vec);
  return len > 0.0f ? vec / len : vec3{};
}

[[nodiscard]] inline auto normalize(quad const& vec) noexcept {
  const float len = length(vec);
  return len > 0.0f ? vec / len : quad{};
}

[[nodiscard]] constexpr auto lerp(vec2 const& a, vec2 const& b, float t) noexcept {
  return a + (b - a) * t;
}

[[nodiscard]] constexpr auto lerp(vec3 const& a, vec3 const& b, float t) noexcept {
  return a + (b - a) * t;
}

[[nodiscard]] constexpr auto lerp(quad const& a, quad const& b, float t) noexcept {
  return a + (b - a) * t;
}

[[nodiscard]] constexpr auto clamp(vec2 const& vec, vec2 const& min, vec2 const& max) noexcept {
  return vec2{
    std::clamp(vec.x, min.x, max.x),
    std::clamp(vec.y, min.y, max.y)
  };
}

[[nodiscard]] constexpr auto clamp(vec3 const& vec, vec3 const& min, vec3 const& max) noexcept {
  return vec3{
    std::clamp(vec.x, min.x, max.x),
    std::clamp(vec.y, min.y, max.y),
    std::clamp(vec.z, min.z, max.z)
  };
}

[[nodiscard]] constexpr auto clamp(quad const& vec, quad const& min, quad const& max) noexcept {
  return quad{
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

[[nodiscard]] constexpr auto min(vec3 const& a, vec3 const& b) noexcept {
  return vec3{
    std::min(a.x, b.x),
    std::min(a.y, b.y),
    std::min(a.z, b.z)
  };
}

[[nodiscard]] constexpr auto min(quad const& a, quad const& b) noexcept {
  return quad{
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

[[nodiscard]] constexpr auto max(vec3 const& a, vec3 const& b) noexcept {
  return vec3{
    std::max(a.x, b.x),
    std::max(a.y, b.y),
    std::max(a.z, b.z)
  };
}

[[nodiscard]] constexpr auto max(quad const& a, quad const& b) noexcept {
  return quad{
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
