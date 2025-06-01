#include "achievement.hpp"

using namespace steam;

#ifdef STEAM
void achievement::unlock(std::string id) {
  if (!SteamUserStats()) {
    return;
  }

  const auto* ptr = id.c_str();

  bool achieved;
  SteamUserStats()->GetAchievement(ptr, &achieved);
  if (achieved) {
    return;
  }

  SteamUserStats()->SetAchievement(ptr);
  SteamUserStats()->StoreStats();
}
#else
void achievement::unlock(std::string) {}
#endif
