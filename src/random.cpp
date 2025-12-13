#include "random.hpp"

namespace {
struct global_initializer final {
  rng::xorshift128plus generator;

  global_initializer() noexcept {
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    generator.seed(static_cast<uint64_t>(now));
  }
};

global_initializer g_init{};
}

namespace rng {
xorshift128plus& global() noexcept {
  return g_init.generator;
}

void global_seed(uint64_t seed) noexcept {
  g_init.generator.seed(seed);
}
}
