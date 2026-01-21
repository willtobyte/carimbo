#include "application.hpp"

application::application(const int argc, char** const argv) noexcept {
  SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);
  std::at_quick_exit([] { SDL_Quit(); });

  PHYSFS_init(argv[0]);
  std::at_quick_exit([] { PHYSFS_deinit(); });

#ifdef HAS_STEAM
  SteamAPI_InitSafe();
  std::at_quick_exit([] { SteamAPI_Shutdown(); });
#endif
}

int application::run() {
  try {
    const auto* const rom = std::getenv("CARTRIDGE");

    filesystem::mount(rom ? rom : "cartridge.rom", "/");

    auto se = scriptengine();
    se.run();
  } catch (const std::exception& e) {
    const auto error = e.what();

    std::println(stderr, "{}", error);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Disaster", error, nullptr);

#ifdef HAS_SENTRY
    const auto event = sentry_value_new_event();
    const auto exc = sentry_value_new_exception("Exception", error);
    sentry_value_set_stacktrace(exc, nullptr, 0);
    sentry_event_add_exception(event, exc);
    sentry_capture_event(event);
    sentry_flush(3000);
#endif

    return 1;
  }

  return 0;
}
