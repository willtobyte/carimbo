#include "achievement.hpp"

using namespace steam;

#ifdef STEAM
void achievement::unlock(std::string id) {
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
void achievement::unlock(std::string) {}
#endif
}
