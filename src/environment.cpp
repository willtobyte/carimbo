#include "environment.hpp"

using namespace platform;

std::optional<std::string> desktop::folder() const noexcept {
  const char* folder = SDL_GetUserFolder(SDL_FOLDER_DESKTOP);
  if (!folder) {
    return std::nullopt;
  }

  return std::string{folder};
}
