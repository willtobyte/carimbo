#include <SDL3/SDL_main.h>

#include "common.hpp"

int main(int argc, char **argv) {
#if defined(NDEBUG) && !defined(EMSCRIPTEN) && !defined(DEVELOPMENT)
  if (auto* out = std::freopen("stdout.txt", "w", stdout)) {
    setvbuf(out, nullptr, _IONBF, 0);
  }

  if (auto* err = std::freopen("stderr.txt", "w", stderr)) {
    setvbuf(err, nullptr, _IONBF, 0);
  }
#endif

  SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);

  PHYSFS_init(argv[0]);

  const auto device = alcOpenDevice(nullptr);
  const auto context = alcCreateContext(device, nullptr);
  alcMakeContextCurrent(context);

#ifdef HAS_STEAM
  SteamAPI_InitSafe();
#endif

  const auto result = application().run();

#ifdef HAS_STEAM
  SteamAPI_Shutdown();
#endif

  alcMakeContextCurrent(nullptr);
  alcDestroyContext(context);
  alcCloseDevice(device);

  PHYSFS_deinit();

  SDL_Quit();

  return result;
}
