#include "application.hpp"

int application::run() {
  try {
    const auto* const rom = std::getenv("CARTRIDGE");

    filesystem::mount(rom ? rom : "cartridge.rom", "/");

    auto se = scriptengine();
    se.run();
  } catch (const std::exception& e) {
    const auto* const error = e.what();

    std::println(stderr, "{}", error);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Disaster", error, nullptr);

#ifdef HAS_SENTRY
    const auto event = sentry_value_new_event();
    const auto exc = sentry_value_new_exception("exception", error);
    sentry_value_set_stacktrace(exc, nullptr, 0);
    sentry_event_add_exception(event, exc);
    sentry_capture_event(event);
    sentry_flush(3000);
#endif

    return 1;
  }

  return 0;
}
