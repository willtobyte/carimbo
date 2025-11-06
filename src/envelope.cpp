#include "envelope.hpp"

using namespace framework;

void envelope::reset(collisionenvelope&& envelope) {
  payload.emplace<collisionenvelope>(std::move(envelope));
}

void envelope::reset(mailenvelope&& envelope) {
  payload.emplace<mailenvelope>(std::move(envelope));
}

void envelope::reset(timerenvelope&& envelope) {
  payload.emplace<timerenvelope>(std::move(envelope));
}

void envelope::reset() {
  payload.emplace<std::monostate>();
}
