#include "application.hpp"

using namespace framework;

application::application(int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER | SDL_INIT_VIDEO);

  PHYSFS_init(argv[0]);

#ifdef STEAM
  SteamAPI_Init();
#endif
}

int application::run() {
  try {
#if SANDBOX
    storage::filesystem::mount("../sandbox", "/");
#else
    storage::filesystem::mount("bundle.7z", "/");
#endif

    auto se = scriptengine();
    se.run();
  } catch (const std::exception &ex) {
    std::cerr << ex.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown error occurred" << std::endl;
    return 1;
  }

  return 0;
}

application::~application() noexcept {
#ifdef STEAM
  SteamAPI_Shutdown();
#endif
  PHYSFS_deinit();
  SDL_Quit();
}
