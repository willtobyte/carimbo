#include "envelope.hpp"

mailenvelope::mailenvelope(std::pmr::memory_resource* mr)
    : to(0), from(0), body(mr) {
  body.reserve(64);
}

void mailenvelope::set(uint64_t to_, uint64_t from_, std::string_view body_) {
  to = to_;
  from = from_;
  body.assign(body_);
}

void mailenvelope::clear() noexcept {
  to = 0;
  from = 0;
  body.clear();
}

void timerenvelope::set(bool repeat_, functor&& fn_) noexcept {
  repeat = repeat_;
  fn = std::move(fn_);
}

void timerenvelope::clear() noexcept {
  repeat = false;
  fn = nullptr;
}

envelope::envelope(std::pmr::memory_resource* mr)
    : _mr(mr), payload(std::in_place_type<mailenvelope>, mr) {}

void envelope::reset(uint64_t to, uint64_t from, std::string_view body) {
  auto* mail = std::get_if<mailenvelope>(&payload);
  if (!mail) {
    payload.emplace<mailenvelope>(_mr);
    mail = std::get_if<mailenvelope>(&payload);
  }
  mail->set(to, from, body);
}

void envelope::reset(bool repeat, functor&& fn) {
  auto* timer = std::get_if<timerenvelope>(&payload);
  if (!timer) {
    payload.emplace<timerenvelope>();
    timer = std::get_if<timerenvelope>(&payload);
  }
  timer->set(repeat, std::move(fn));
}

void envelope::reset() noexcept {
  if (auto* mail = std::get_if<mailenvelope>(&payload)) {
    mail->clear();
  } else if (auto* timer = std::get_if<timerenvelope>(&payload)) {
    timer->clear();
  }
}

const mailenvelope* envelope::try_mail() const noexcept {
  return std::get_if<mailenvelope>(&payload);
}

const timerenvelope* envelope::try_timer() const noexcept {
  return std::get_if<timerenvelope>(&payload);
}
