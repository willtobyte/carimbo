#include <SDL3/SDL_main.h>

#include "application.hpp"

int main(int argc, char **argv) {
#if defined(NDEBUG) && !defined(EMSCRIPTEN) && !defined(DEVELOPMENT)
  if (auto* out = std::freopen("stdout.txt", "w", stdout)) {
    setvbuf(out, nullptr, _IONBF, 0);
  }

  if (auto* err = std::freopen("stderr.txt", "w", stderr)) {
    setvbuf(err, nullptr, _IONBF, 0);
  }
#endif

  application app(argc, argv);

  const auto code = app.run();

  std::quick_exit(code);
}
