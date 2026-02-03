#include "achievement.hpp"
#include "steam.hpp"

void achievement::unlock(std::string_view id) noexcept {
#ifdef HAS_STEAM
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
#endif
}
