#include "postalservice.hpp"

using namespace framework;

postalservice::postalservice() noexcept
    : _mail_pool(mail_pool::instance()) {
  _mail_pool->reserve(1000);
}

void postalservice::post(const mail& message) noexcept {
  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::mail);
  event.user.data1 = new mail(message);

  SDL_PushEvent(&event);
}
