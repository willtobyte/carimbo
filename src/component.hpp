#pragma once

#include "common.hpp"

#include "geometry.hpp"

using componenttype = std::uint8_t;

constexpr componenttype component_n = 32;

using signature = std::bitset<component_n>;

struct alignas(8) transform final {
	vec2 position;
	double angle;
	float scale;

	[[nodiscard]] constexpr bool operator==(const transform&) const noexcept = default;
};

struct alignas(4) tint final {
  uint8_t r{0};
  uint8_t g{0};
  uint8_t b{0};
  uint8_t a{255};

  [[nodiscard]] constexpr bool operator==(const tint&) const noexcept = default;
};

struct alignas(16) timeline final {
  static constexpr auto capacity = 16;

  std::array<char, 64> action;
  std::array<char, 64> next;
  std::array<quad, capacity> frames;
  std::array<uint16_t, capacity> durations;
  uint16_t count{0};
  uint16_t current{0};
  uint64_t tick{0};

  [[nodiscard]] constexpr bool operator==(const timeline&) const noexcept = default;
};

struct alignas(8) sprite final {
  size_t hash;
  reflection reflection;

  [[nodiscard]] constexpr bool operator==(const sprite&) const noexcept = default;
};

struct alignas(4) physics final {
	b2BodyId id;
	b2BodyType type;
	bool enabled;

	[[nodiscard]] bool operator==(const physics& other) const noexcept {
		return std::memcmp(this, &other, sizeof(physics)) == 0;
	};
};

static_assert(std::is_trivially_copyable_v<physics>);
static_assert(alignof(physics) >= alignof(b2BodyId));
static_assert(alignof(physics) >= alignof(b2BodyType));
