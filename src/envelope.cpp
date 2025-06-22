#include "envelope.hpp"

using namespace framework;

void envelope::reset(collisionenvelope&& envelope) noexcept {
  payload.emplace<collisionenvelope>(std::move(envelope));
}

void envelope::reset(mailenvelope&& envelope) noexcept {
  payload.emplace<mailenvelope>(std::move(envelope));
}

void envelope::reset(timerenvelope&& envelope) noexcept {
  payload.emplace<timerenvelope>(std::move(envelope));
}

void envelope::reset() noexcept {
  payload.emplace<std::monostate>();
}
