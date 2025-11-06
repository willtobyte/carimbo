#include "environment.hpp"

using namespace platform;

std::optional<std::string> desktop::folder() const {
  const auto* folder = SDL_GetUserFolder(SDL_FOLDER_DESKTOP);
  if (!folder) {
    return std::nullopt;
  }

  return std::string{folder};
}

int32_t operatingsystem::compute() const {
  return SDL_GetNumLogicalCPUCores();
}

int32_t operatingsystem::memory() const {
  return SDL_GetSystemRAM();
}

std::string operatingsystem::name() const {
  return std::string{SDL_GetPlatform()};
}
