#include "achievement.hpp"

using namespace steam;

void achievement::unlock(std::string_view id) noexcept {
#ifndef HAS_STEAM
  return;
#endif

  if (!SteamUserStats()) [[unlikely]] {
    return;
  }

  const auto* ptr = id.data();

  if (GetAchievement(ptr)) [[unlikely]] {
    return;
  }

  std::println("[achievement] unlocked {}", id);

  SetAchievement(ptr);
  StoreStats();
}
