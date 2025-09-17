#include "environment.hpp"

using namespace platform;

std::optional<std::string> desktop::folder() const noexcept {
  const auto* folder = SDL_GetUserFolder(SDL_FOLDER_DESKTOP);
  if (!folder) {
    return std::nullopt;
  }

  return std::string{folder};
}

int32_t operatingsystem::compute() const noexcept {
  return SDL_GetNumLogicalCPUCores();
}

int32_t operatingsystem::memory() const noexcept {
  return SDL_GetSystemRAM();
}

std::string operatingsystem::name() const noexcept {
  return std::string{SDL_GetPlatform()};
}
