#pragma once

#include "common.hpp"

using entity = uint64_t;

struct entitystorage final {
  static constexpr size_t size = 10000ULL;
  static constexpr size_t alignment = 64;

  alignas(alignment) entity data[size];

  [[nodiscard]] constexpr entity* begin() noexcept { return data; }
  [[nodiscard]] constexpr entity* end() noexcept { return data + size; }
  [[nodiscard]] constexpr const entity* begin() const noexcept { return data; }
  [[nodiscard]] constexpr const entity* end() const noexcept { return data + size; }

  [[nodiscard]] constexpr entity& operator[](size_t index) noexcept { return data[index]; }
  [[nodiscard]] constexpr const entity& operator[](size_t index) const noexcept { return data[index]; }
};

struct alignas(8) transform final {
	vec2 position;
	float angle;
	float scale;

	[[nodiscard]] constexpr bool operator==(const transform&) const noexcept = default;
};

struct alignas(4) alpha final {
	uint8_t value{255};

	[[nodiscard]] constexpr bool operator==(const alpha&) const noexcept = default;
};

struct alignas(8) physics final {
	b2BodyId id;
	b2BodyType type;
	bool enabled;

	[[nodiscard]] bool operator==(const physics& other) const noexcept {
		return std::memcmp(this, &other, sizeof(physics)) == 0;
	}
};

static_assert(sizeof(transform) == 16);
static_assert(alignof(transform) == 8);
static_assert(std::is_trivially_copyable_v<transform>);
static_assert(std::is_standard_layout_v<transform>);

using componenttype = uint8_t;

struct componentstorage final {
  static constexpr size_t size = 256ULL;
  static constexpr size_t alignment = 64;

  alignas(alignment) componenttype data[size];

  [[nodiscard]] constexpr componenttype* begin() noexcept { return data; }
  [[nodiscard]] constexpr componenttype* end() noexcept { return data + size; }
  [[nodiscard]] constexpr const componenttype* begin() const noexcept { return data; }
  [[nodiscard]] constexpr const componenttype* end() const noexcept { return data + size; }

  [[nodiscard]] constexpr componenttype& operator[](size_t idx) noexcept { return data[idx]; }
  [[nodiscard]] constexpr const componenttype& operator[](size_t idx) const noexcept { return data[idx]; }
};

using signature = std::bitset<componentstorage::size>;

namespace signature_ops {
  [[nodiscard]] inline constexpr bool matches(const signature& signature, const signature& required) noexcept {
    return (signature & required) == required;
  }

  [[nodiscard]] inline constexpr signature combine(const signature& a, const signature& b) noexcept {
    return a | b;
  }
}
