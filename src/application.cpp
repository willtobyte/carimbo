#include "application.hpp"

using namespace framework;

application::application(int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);

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
  } catch (const std::exception &e) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(), nullptr);
    return 1;
  }

  return 0;
}

application::~application() {
  PHYSFS_deinit();
  SDL_Quit();
}
