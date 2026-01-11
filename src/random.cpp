#include "random.hpp"

namespace {
struct random final {
  rng::xorshift128plus generator;

  random() noexcept {
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    generator.seed(static_cast<uint64_t>(now));
  }
};

random _random{};
}

namespace rng {
xorshift128plus& global() noexcept {
  return _random.generator;
}

void seed(uint64_t value) noexcept {
  _random.generator.seed(value);
}
}
