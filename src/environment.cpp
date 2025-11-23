#include "environment.hpp"

std::optional<std::string_view> desktop::folder() const noexcept {
  const auto* const folder = SDL_GetUserFolder(SDL_FOLDER_DESKTOP);
  if (!folder) {
    return std::nullopt;
  }

  return folder;
}

int operatingsystem::compute() const noexcept {
  return SDL_GetNumLogicalCPUCores();
}

int operatingsystem::memory() const noexcept {
  return SDL_GetSystemRAM();
}

std::string_view operatingsystem::name() const noexcept {
  return SDL_GetPlatform();
}
