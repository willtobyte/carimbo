#include "postalservice.hpp"

postalservice::postalservice()
    : _envelopepool(envelopepool::instance()) {
}

void postalservice::post(const mail& message) {
  auto envelope = _envelopepool.acquire(mailenvelope(message.to, message.kind, message.body));

  SDL_Event event{};
  event.type = static_cast<uint32_t>(event::type::mail);
  event.user.data1 = envelope.release();

  SDL_PushEvent(&event);
}