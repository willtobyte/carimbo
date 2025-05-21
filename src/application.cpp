#include "application.hpp"

using namespace framework;

[[noreturn]] void fail() {
  if (const auto ptr = std::current_exception()) {
    const char* error = nullptr;

    try {
      std::rethrow_exception(ptr);
    } catch (const std::bad_exception&) {
    } catch (const std::exception &e) {
      error = e.what();
    } catch (...) {
      error = "Unhandled unknown exception";
    }

    if (error) {
      #ifdef DEBUG
        #if defined(_MSC_VER)
          __debugbreak();
        #else
          raise(SIGTRAP);
        #endif
      #endif

      #ifdef HAVE_STACKSTRACE
        boost::stacktrace::stacktrace st;
        fmt::println(stderr, "Stack trace:\n{}\n", boost::stacktrace::to_string(st));
      #endif

      fmt::println(stderr, "{}", error);

      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Catastrophe", error, nullptr);
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

  #ifdef ANDROID
  PHYSFS_init(SDL_AndroidGetInternalStoragePath());
  #else
  PHYSFS_init(argv[0]);
  #endif
}

int32_t application::run() {
#if SANDBOX
  storage::filesystem::mount("../sandbox", "/");
#else
  std::unique_ptr<char, SDLDeleter> base(SDL_GetBasePath(), SDL_free);
  const auto path = fmt::format("{}bundle.7z", base.get());
  SDL_free(base);
  storage::filesystem::mount(path.c_str(), "/");
#endif

  auto se = scriptengine();
  se.run();

  return 0;
}
