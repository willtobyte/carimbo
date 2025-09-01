#include "environment.hpp"

using namespace platform;

std::optional<std::string> desktop::path() const noexcept {
  const char* path = SDL_GetUserFolder(SDL_FOLDER_DESKTOP);
  if (!path) {
    return std::nullopt;
  }

  return std::string{path};
}
