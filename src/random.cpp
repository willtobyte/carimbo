#include "random.hpp"

namespace {
struct random final {
  rng::xorshift128plus generator;

  explicit random(uint64_t extra) noexcept {
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    generator.seed(static_cast<uint64_t>(now) ^ extra);
  }
};

random _engine{0};
random _script{rng::mix_constant};
}

namespace rng::engine {
xorshift128plus& global() noexcept {
  return _engine.generator;
}

void seed(uint64_t value) noexcept {
  _engine.generator.seed(value);
}
}

namespace rng::script {
xorshift128plus& global() noexcept {
  return _script.generator;
}

void seed(uint64_t value) noexcept {
  _script.generator.seed(value);
}
}
