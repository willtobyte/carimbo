#pragma once

#include "common.hpp"

#include "geometry.hpp"

using entity = std::uint64_t;

constexpr entity capacity = 4096;

struct alignas(8) transform final {
	vec2 position;
	double angle;
	float scale;
};

struct alignas(4) tint final {
  uint8_t r{0};
  uint8_t g{0};
  uint8_t b{0};
  uint8_t a{255};
};

struct alignas(16) timeline final {
  static constexpr auto capacity = 32;

  std::array<box2, capacity> frames;
  std::array<uint16_t, capacity> durations;
  uint16_t count = 0;
};
