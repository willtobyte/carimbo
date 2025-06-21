#include "timer.hpp"

using namespace framework;

timer::timer() noexcept
    : repeat(false) {}

timer::timer(bool repeat, std::function<void()> fn) noexcept
    : repeat(repeat), fn(std::move(fn)) {}

void timer::reset() noexcept {
  repeat = false;
  fn = nullptr;
}

void timer::reset(bool repeat, std::function<void()> fn) noexcept {
  this->repeat = repeat;
  this->fn = fn;
}
