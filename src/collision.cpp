#include "collision.hpp"

using namespace framework;

collision::collision() noexcept
    : a{0},
      b{0} {}

collision::collision(uint64_t a, uint64_t b) noexcept
    : a(a),
      b(b) {}

void collision::reset() noexcept {
  a = 0;
  b = 0;
}

void collision::reset(uint64_t a, uint64_t b) noexcept {
  this->a = a;
  this->b = b;
}
