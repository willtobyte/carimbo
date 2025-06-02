#include "achievement.hpp"

using namespace steam;

#ifdef HAVE_STEAM
void achievement::unlock(std::string id) {
  if (!SteamUserStats()) {
    fmt::print("NOT SteamUserStats");
    return;
  }

  const auto* ptr = id.c_str();

  if (GetAchievement(ptr)) {
    fmt::println("achieved!");
    return;
  }

  SetAchievement(ptr);
  StoreStats();
  fmt::print("DONE");
}
#else
void achievement::unlock(std::string) {}
#endif
