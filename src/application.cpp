#include "application.hpp"

using namespace framework;

[[noreturn]] void fail() {
  const char* error = nullptr;

  if (const auto ptr = std::current_exception()) {
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

      fmt::println(stderr, "{}", error);
      #ifdef HAVE_BOOST
      boost::stacktrace::stacktrace st;
      fmt::println(stderr, "Stack trace:\n{}\n", boost::stacktrace::to_string(st));
      #endif

      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Catastrophe", error, nullptr);
    }
  }

  std::abort();
}

application::application(int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  std::set_terminate(fail);

  SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);

  PHYSFS_init(argv[0]);
}

int32_t application::run() {
#if SANDBOX
  storage::filesystem::mount("../sandbox", "/");
#else
  storage::filesystem::mount("bundle.7z", "/");
#endif

  auto se = scriptengine();
  se.run();

  return 0;
}

application::~application() {
  PHYSFS_deinit();
  SDL_Quit();
}
