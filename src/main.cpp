#include "application.hpp"

int main(int argc, char **argv) {
  SDL_SetMainReady();

  framework::application app(argc, std::move(argv));

  return app.run();
}
