#include "achievement.hpp"

namespace framework {
#ifdef STEAM
void unlockachievement(const std::string &id) {
  if (!SteamUserStats())
    return;

  auto achieved = false;
  SteamUserStats()->GetAchievement(id, &achieved);
  if (achieved)
    return;

  SteamUserStats()->SetAchievement(id);
  SteamUserStats()->StoreStats();
}
#else
void unlockachievement(const std::string &) {}
#endif
}
