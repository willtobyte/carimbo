#include "envelope.hpp"

using namespace framework;

void envelope::reset(uint64_t a, uint64_t b) noexcept {
  kind = envelopekind::collision;
  payload = collision_t{a, b};
}

void envelope::reset(uint64_t to, std::string&& kind_str, std::string&& body_str) noexcept {
  kind = envelopekind::mail;
  payload = mail_t{to, std::move(kind_str), std::move(body_str)};
}

void envelope::reset(bool repeat, std::function<void()>&& fn) noexcept {
  kind = envelopekind::timer;
  payload = timer_t{repeat, std::move(fn)};
}

void envelope::reset() noexcept {
  kind = envelopekind::none;
  payload = std::monostate{};
}
