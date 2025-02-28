#include "postalservice.hpp"

using namespace framework;

void postalservice::post(const mail &message) noexcept {
  SDL_Event event{};
  event.type = input::eventtype::mail;
  event.user.data1 = new mail(message);

  SDL_PushEvent(&event);
}
