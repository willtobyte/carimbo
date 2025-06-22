#include "envelope.hpp"

using namespace framework;

void envelope::reset(uint64_t a, uint64_t b) noexcept {
  payload.emplace<collision_t>(a, b);
}

void envelope::reset(uint64_t to, std::string&& kind, std::string&& body) noexcept {
  payload.emplace<mail_t>(to, std::move(kind), std::move(body));
}

void envelope::reset(bool repeat, std::function<void()>&& fn) noexcept {
  payload.emplace<timer_t>(repeat, std::move(fn));
}

void envelope::reset() noexcept {
  payload.emplace<std::monostate>();
}
