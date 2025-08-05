#include "application.hpp"
#include <stdexcept>

using namespace framework;

[[noreturn]] void fail() {
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
      #ifdef HAVE_STACKSTRACE
        boost::stacktrace::stacktrace st;
        std::println(stderr, "Stack trace:\n{}\n", boost::stacktrace::to_string(st));
      #endif

      std::println(stderr, "{}", error);

      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Catastrophe", error, nullptr);

      #ifdef DEBUG
        #if defined(_MSC_VER)
          __debugbreak();
        #else
          raise(SIGTRAP);
        #endif
      #endif
    }
  }

  std::exit(EXIT_FAILURE);
}

application::application(int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  std::set_terminate(fail);

  constexpr const auto fn = [](int) {
    std::exit(EXIT_SUCCESS);
  };

  std::signal(SIGINT, fn);
  std::signal(SIGTERM, fn);

  std::atexit([] { PHYSFS_deinit(); });
  std::atexit([] { SDL_Quit(); });

  SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);
  PHYSFS_init(argv[0]);
  #ifdef HAVE_STEAM
  SteamAPI_InitSafe();
  #endif
}

int32_t application::run() {
  #if defined(EMSCRIPTEN) || !defined(SANDBOX)
  storage::filesystem::mount("cartridge.zip", "/");
  #else
  storage::filesystem::mount("../sandbox", "/");
  #endif

  auto se = scriptengine();
  se.run();

  return 0;
}
