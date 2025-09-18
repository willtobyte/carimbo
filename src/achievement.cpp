#include "achievement.hpp"

using namespace steam;

#ifdef HAVE_STEAM
void achievement::unlock(const std::string& id) noexcept {
  if (!SteamUserStats()) {
    return;
  }

  const auto* ptr = id.c_str();

  if (GetAchievement(ptr)) {
    return;
  }

  SetAchievement(ptr);
  StoreStats();

  std::println("[achievement] unlocked: {}", id);
}
#else
void achievement::unlock(const std::string& id) noexcept {
  std::println("[achievement] unlocked: {}", id);
}
#endif
