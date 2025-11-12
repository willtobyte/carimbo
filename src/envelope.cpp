#include "envelope.hpp"

using namespace framework;

collisionenvelope::collisionenvelope(const uint64_t a, const uint64_t b) noexcept
  : a(a), b(b) {}

mailenvelope::mailenvelope(const uint64_t to, const std::string_view kind, const std::string_view body)
  : to(to), kind(kind), body(body) {}

timerenvelope::timerenvelope(const bool repeat, std::function<void()>&& fn) noexcept
  : repeat(repeat), fn(std::move(fn)) {}

void envelope::reset(collisionenvelope&& envelope) noexcept {
  payload.emplace<collisionenvelope>(std::move(envelope));
}

void envelope::reset(mailenvelope&& envelope) {
  payload.emplace<mailenvelope>(std::move(envelope));
}

void envelope::reset(timerenvelope&& envelope) noexcept {
  payload.emplace<timerenvelope>(std::move(envelope));
}

void envelope::reset() noexcept {
  payload.emplace<std::monostate>();
}

const collisionenvelope* envelope::try_collision() const noexcept {
  return std::get_if<collisionenvelope>(&payload);
}

const mailenvelope* envelope::try_mail() const noexcept {
  return std::get_if<mailenvelope>(&payload);
}

const timerenvelope* envelope::try_timer() const noexcept {
  return std::get_if<timerenvelope>(&payload);
}
