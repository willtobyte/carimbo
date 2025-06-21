#include "postalservice.hpp"

using namespace framework;

postalservice::postalservice() noexcept
    : _mailpool(mailpool::instance()) {
}

void postalservice::post(const mail& message) noexcept {
  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::mail);
  event.user.data1 = _mailpool->acquire(message).release();

  SDL_PushEvent(&event);
}
