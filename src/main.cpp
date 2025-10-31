#include <SDL3/SDL_main.h>

#include "application.hpp"

int main(int argc, char **argv) {
#ifdef NDEBUG
#ifndef EMSCRIPTEN
#ifndef DEVELOPMENT
  auto* out = std::freopen("stdout.txt", "w", stdout);
  if (out) setvbuf(out, nullptr, _IONBF, 0);

  auto* err = std::freopen("stderr.txt", "w", stderr);
  if (err) setvbuf(err, nullptr, _IONBF, 0);
#endif
#endif
#endif

  framework::application app(argc, argv);
  return app.run();
}
