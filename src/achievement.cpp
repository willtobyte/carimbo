#include "achievement.hpp"

using namespace steam;

#ifdef HAVE_STEAM
void achievement::unlock(const std::string& id) {
  if (!SteamUserStats()) {
    return;
  }

  const auto* ptr = id.c_str();

  if (GetAchievement(ptr)) {
    return;
  }

  std::println("[achievement] unlocked {}", id);

  SetAchievement(ptr);
  StoreStats();
}
#else
void achievement::unlock(const std::string&) {}
#endif
