#include "application.hpp"

using namespace framework;

application::application(int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER | SDL_INIT_VIDEO);

  PHYSFS_init(argv[0]);
}

int32_t application::run() {
  try {
#if SANDBOX
    storage::filesystem::mount("../sandbox", "/");
#else
    storage::filesystem::mount("bundle.7z", "/");
#endif

    auto se = scriptengine();
    se.run();
  } catch (const std::exception &ex) {
    fmt::println(stderr, "{}", ex.what());
    return 1;
  } catch (...) {
    fmt::println(stderr, "Unknown error occurred");
    return 3;
  }

  return 0;
}

application::~application() noexcept {
  PHYSFS_deinit();
  SDL_Quit();
}
