#include "envelope.hpp"

mailenvelope::mailenvelope(std::pmr::memory_resource* mr)
  : to(0), kind(mr), body(mr) {
  kind.reserve(32);
  body.reserve(256);
}

mailenvelope::mailenvelope(const uint64_t to, const std::string_view kind, const std::string_view body, std::pmr::memory_resource* mr)
  : to(to), kind(kind, mr), body(body, mr) {}

void mailenvelope::clear() noexcept {
  to = 0;
  kind.clear();
  body.clear();
}

timerenvelope::timerenvelope(const bool repeat, std::function<void()>&& fn) noexcept
  : repeat(repeat), fn(std::move(fn)) {}

timerenvelope::timerenvelope() noexcept
  : repeat(false), fn(nullptr) {}

void timerenvelope::clear() noexcept {
  repeat = false;
  fn = nullptr;
}

envelope::envelope(std::pmr::memory_resource* mr)
  : _mr(mr), payload(std::monostate{}) {}

void envelope::reset(mailenvelope&& envelope) {
  if (auto* current = std::get_if<mailenvelope>(&payload)) {
    current->to = envelope.to;
    current->kind = std::move(envelope.kind);
    current->body = std::move(envelope.body);
  } else {
    payload.emplace<mailenvelope>(envelope.to, envelope.kind, envelope.body, _mr);
  }
}

void envelope::reset(timerenvelope&& envelope) noexcept {
  payload.emplace<timerenvelope>(std::move(envelope));
}

void envelope::reset() noexcept {
  if (auto* mail = std::get_if<mailenvelope>(&payload)) {
    mail->clear();
  } else if (auto* timer = std::get_if<timerenvelope>(&payload)) {
    timer->clear();
  }

  payload.emplace<std::monostate>();
}

const mailenvelope* envelope::try_mail() const noexcept {
  return std::get_if<mailenvelope>(&payload);
}

const timerenvelope* envelope::try_timer() const noexcept {
  return std::get_if<timerenvelope>(&payload);
}
