#include "application.hpp"

using namespace framework;

application::application(int argc, char **argv) {
  constexpr const auto fn = [](int) {
    std::exit(EXIT_SUCCESS);
  };

  std::signal(SIGINT, fn);
  std::signal(SIGTERM, fn);

#ifdef HAVE_SENTRY
  std::atexit([] { sentry_close(); });
#endif

  std::atexit([] { PHYSFS_deinit(); });
  std::atexit([] { SDL_Quit(); });

  SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);
  PHYSFS_init(argv[0]);
#ifdef HAVE_STEAM
  SteamAPI_InitSafe();
#endif
}

int32_t application::run() {
  static_assert(std::endian::native == std::endian::little);

  try {
    const auto* p = std::getenv("CARTRIDGE");

    storage::filesystem::mount(p ? p : "cartridge.rom", "/");

    auto se = scriptengine();
    se.run();
  } catch (const std::exception& e) {
    const auto* error = e.what();

#ifdef HAVE_SENTRY
    const auto ev = sentry_value_new_event();

    const auto exc = sentry_value_new_exception("exception", error);
    sentry_event_add_exception(ev, exc);
    sentry_capture_event(ev);
#endif

    std::println(stderr, "{}", error);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Disaster", error, nullptr);
  }

  return 0;
}
