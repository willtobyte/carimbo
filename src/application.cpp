#include "application.hpp"

using namespace framework;

[[noreturn]] static void fail() {
  std::cout.flush();
  std::cerr.flush();

  if (const auto ptr = std::current_exception()) {
    const char* error = nullptr;

    try {
      std::rethrow_exception(ptr);
    } catch (const std::bad_exception&) {
    } catch (const std::exception& e) {
      error = e.what();
    } catch (...) {
      error = "Unhandled unknown exception";
    }

    if (error) {
      std::fflush(stdout);

      #ifdef HAVE_SENTRY
      const auto ev = sentry_value_new_event();

      const auto exc = sentry_value_new_exception("exception", error);
      sentry_event_add_exception(ev, exc);

      sentry_capture_event(ev);
      #endif

      #ifdef HAVE_BOOST
      boost::stacktrace::stacktrace st;
      std::println(stderr, "{}", boost::stacktrace::to_string(st));
      #endif

      std::println(stderr, "{}", error);

      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Disaster", error, nullptr);

      #ifdef DEBUG
        #ifdef _MSC_VER
          __debugbreak();
        #else
          raise(SIGTRAP);
        #endif
      #endif
    }
  }

  std::exit(EXIT_FAILURE);
}

application::application(int argc, char** argv) {
  dup2(open("stdout.txt", O_RDWR | O_CREAT | O_TRUNC, 0600), STDOUT_FILENO);
  dup2(open("stderr.txt", O_RDWR | O_CREAT | O_TRUNC, 0600), STDERR_FILENO);

  std::set_terminate(fail);

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

  const auto* p = std::getenv("CARTRIDGE");

  storage::filesystem::mount(p ? p : "cartridge.zip", "/");

  auto se = scriptengine();
  se.run();

  return 0;
}
