#include "achievement.hpp"

namespace framework {
#ifdef STEAM
void unlockachievement(const std::string &id) {
  if (!SteamUserStats())
    return;

  auto achieved = false;
  SteamUserStats()->GetAchievement(id.c_str(), &achieved);
  if (achieved)
    return;

  SteamUserStats()->SetAchievement(id.c_str());
  SteamUserStats()->StoreStats();
}
#else
void unlockachievement(const std::string &) {}
#endif
}
