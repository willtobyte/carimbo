#include "postalservice.hpp"

using namespace framework;

postalservice::postalservice() noexcept
    : _envelopepool(envelopepool::instance()) {
}

void postalservice::post(const mail& message) noexcept {
  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::mail);
  event.user.data1 = _envelopepool->acquire(message.to, message.kind, message.body).release();

  SDL_PushEvent(&event);
}
