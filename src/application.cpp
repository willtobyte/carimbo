#include "application.hpp"

using namespace framework;

application::application(const int argc, char** const argv) {
#ifdef HAVE_SENTRY
  std::atexit([] { sentry_flush(3000); }); // sentry_close();
#endif

#ifdef HAVE_STEAM
  std::atexit([] { SteamAPI_Shutdown(); });
#endif

  std::atexit([] { PHYSFS_deinit(); });
  std::atexit([] { SDL_Quit(); });

  SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);
  PHYSFS_init(argv[0]);
#ifdef HAVE_STEAM
  SteamAPI_InitSafe();
#endif
}

int application::run() {
  static_assert(std::endian::native == std::endian::little);

  try {
    const char* const p = std::getenv("CARTRIDGE");

    storage::filesystem::mount(p ? p : "cartridge.rom", "/");

    auto se = scriptengine();
    se.run();
  } catch (const std::exception& e) {
    const auto error = e.what();

    std::println(stderr, "{}", error);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Disaster", error, nullptr);

#ifdef HAVE_SENTRY
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
