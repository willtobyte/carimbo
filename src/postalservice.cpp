#include "postalservice.hpp"

using namespace framework;

postalservice::postalservice() noexcept
    : _mail_pool(mail_pool::instance()) {
  _mail_pool->reserve(1000);
}

void postalservice::post(const mail& message) noexcept {
  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::mail);
  const auto mail = _mail_pool->acquire(message);
  event.user.data1 = mail.get();

  SDL_PushEvent(&event);
}
