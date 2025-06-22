#include "envelope.hpp"

using namespace framework;

void envelope::reset(uint64_t a, uint64_t b) noexcept {
  payload = collision_t{a, b};
}

void envelope::reset(uint64_t to, std::string&& kind, std::string&& body) noexcept {
  payload = mail_t{to, std::move(kind), std::move(body)};
}

void envelope::reset(bool repeat, std::function<void()>&& fn) noexcept {
  payload = timer_t{repeat, std::move(fn)};
}

void envelope::reset() noexcept {
  payload = std::monostate{};
}
