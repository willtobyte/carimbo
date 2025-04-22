#include "application.hpp"

using namespace framework;

[[noreturn]] void fail() {
  const char* error = nullptr;

  try {
    std::rethrow_exception(std::current_exception());
  } catch (const std::exception& e) {
    error = e.what();
  } catch (...) {
    error = "Unhandled unknown exception";
  }

  fmt::println(stderr, "{}", error);
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Catastrophe", error, nullptr);

  std::abort();
}

application::application(int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  std::set_terminate(fail);

  SDL_Init(SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);

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
