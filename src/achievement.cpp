#include "achievement.hpp"

using namespace steam;

#ifdef HAVE_STEAM
void achievement::unlock(std::string id) {
  if (!SteamUserStats()) {
    fmt::print("NOT SteamUserStats");
    return;
  }

  const auto* ptr = id.c_str();

  bool achieved;
  SteamUserStats()->GetAchievement(ptr, &achieved);
  if (achieved) {
    fmt::println("achieved!");
    return;
  }

  SteamUserStats()->SetAchievement(ptr);
  SteamUserStats()->StoreStats();
  fmt::print("DONE");
}
#else
void achievement::unlock(std::string) {}
#endif
