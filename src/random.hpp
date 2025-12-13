#pragma once

#include "common.hpp"

namespace rng {
inline constexpr uint64_t mix_constant = 0xdeadbeefcafebabeULL;
inline constexpr double inv_max_u64 = 1.0 / static_cast<double>(std::numeric_limits<uint64_t>::max());
inline constexpr float inv_max_u64f = 1.0f / static_cast<float>(std::numeric_limits<uint64_t>::max());

class xorshift128plus final {
public:
  using result_type = uint64_t;

  [[nodiscard]] static constexpr result_type min() noexcept { return 0; }
  [[nodiscard]] static constexpr result_type max() noexcept { return std::numeric_limits<uint64_t>::max(); }

  constexpr xorshift128plus() noexcept = default;

  constexpr explicit xorshift128plus(uint64_t seed) noexcept {
    this->seed(seed);
  }

  constexpr void seed(uint64_t value) noexcept {
    const auto safe = value == 0 ? 1ULL : value;
    _state[0] = safe;
    _state[1] = safe ^ mix_constant;
  }

  [[nodiscard]] constexpr result_type operator()() noexcept {
    const auto s1 = _state[0];
    const auto s0 = _state[1];
    const auto result = s0 + s1;
    const auto t = s1 ^ (s1 << 23);

    _state[0] = s0;
    _state[1] = t ^ s0 ^ (t >> 18) ^ (s0 >> 5);

    return result;
  }

  [[nodiscard]] constexpr double uniform() noexcept {
    return static_cast<double>((*this)()) * inv_max_u64;
  }

  [[nodiscard]] constexpr float uniformf() noexcept {
    return static_cast<float>((*this)()) * inv_max_u64f;
  }

  template<std::integral T>
  [[nodiscard]] constexpr T range(T low, T high) noexcept {
    if (low >= high) [[unlikely]] {
      return low;
    }
    const auto ulow = static_cast<uint64_t>(low);
    const auto range_size = static_cast<uint64_t>(high - low + 1);
    return static_cast<T>(ulow + ((*this)() % range_size));
  }

  template<std::floating_point T>
  [[nodiscard]] constexpr T range(T low, T high) noexcept {
    if constexpr (std::is_same_v<T, float>) {
      return low + (high - low) * uniformf();
    } else {
      return low + (high - low) * static_cast<T>(uniform());
    }
  }

private:
  alignas(16) std::array<uint64_t, 2> _state{};
};

template<std::floating_point T>
struct uniform_real final {
  T a{0};
  T b{1};

  constexpr uniform_real() noexcept = default;
  constexpr uniform_real(T min, T max) noexcept : a(min), b(max) {}

  template<typename G>
  [[nodiscard]] constexpr T operator()(G& g) noexcept {
    if constexpr (std::is_same_v<T, float>) {
      return a + (b - a) * g.uniformf();
    } else {
      return a + (b - a) * static_cast<T>(g.uniform());
    }
  }

  [[nodiscard]] constexpr T min() const noexcept { return a; }
  [[nodiscard]] constexpr T max() const noexcept { return b; }
};

template<std::integral T>
struct uniform_int final {
  T a{0};
  T b{std::numeric_limits<T>::max()};

  constexpr uniform_int() noexcept = default;
  constexpr uniform_int(T min, T max) noexcept : a(min), b(max) {}

  template<typename G>
  [[nodiscard]] constexpr T operator()(G& g) noexcept {
    return g.template range<T>(a, b);
  }

  [[nodiscard]] constexpr T min() const noexcept { return a; }
  [[nodiscard]] constexpr T max() const noexcept { return b; }
};

xorshift128plus& global() noexcept;
void global_seed(uint64_t seed) noexcept;
}
