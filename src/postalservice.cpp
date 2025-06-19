#include "postalservice.hpp"

using namespace framework;

void postalservice::post(const mail& message) noexcept {
  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::mail);
  event.user.data1 = new mail(message);

  SDL_PushEvent(&event);
}
