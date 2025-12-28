#include "postalservice.hpp"

#include "envelope.hpp"
#include "event.hpp"
#include "objectpool.hpp"

postalservice::postalservice()
    : _envelopepool(envelopepool::instance()) {}

void postalservice::post(const mail& message) {
  auto envelope = _envelopepool.acquire();
  envelope->reset(message.to, message.from, message.body);

  SDL_Event event{};
  event.type = static_cast<uint32_t>(event::type::mail);
  event.user.data1 = envelope.release();

  SDL_PushEvent(&event);
}
