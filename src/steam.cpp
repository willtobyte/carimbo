#include "steam.hpp"

#ifdef _WIN32
#include <windows.h>

#ifndef S_CALLTYPE
#define S_CALLTYPE __cdecl
#endif

static const HMODULE steamdll = LoadLibraryA("steam_api64.dll");

bool SteamAPI_InitSafe() {
  fmt::println("SteamAPI_InitSafe >>> ");
  if (!steamdll) {
    fmt::println("Not steamdll");
    return false;
  }

  const auto address = GetProcAddress(steamdll, "SteamAPI_InitSafe");
  if (!address) {
    fmt::println("Not SteamAPI_InitSafe");
    return false;
  }

  using SteamAPI_InitSafe_t = bool(S_CALLTYPE *)();
  return reinterpret_cast<SteamAPI_InitSafe_t>(address)();
}

void SteamAPI_Shutdown() {
  if (!steamdll) {
    return;
  }

  const auto address = GetProcAddress(steamdll, "SteamAPI_Shutdown");
  if (!address) {
    return;
  }

  using SteamAPI_Shutdown_t = void(S_CALLTYPE *)();
  reinterpret_cast<SteamAPI_Shutdown_t>(address)();
}

void SteamAPI_RunCallbacks() {
  if (!steamdll) {
    return;
  }

  const auto address = GetProcAddress(steamdll, "SteamAPI_RunCallbacks");
  if (!address) {
    return;
  }

  using SteamAPI_RunCallbacks_t = void(S_CALLTYPE *)();
  reinterpret_cast<SteamAPI_RunCallbacks_t>(address)();
}

void* SteamUserStats() {
  if (!steamdll) {
    return nullptr;
  }

  const auto address = GetProcAddress(steamdll, "SteamUserStats");
  if (!address) {
    return nullptr;
  }

  using SteamUserStats_t = void*(S_CALLTYPE *)();
  return reinterpret_cast<SteamUserStats_t>(address)();
}

bool GetAchievement(const char* name) {
  const auto stats = SteamUserStats();
  if (!stats) {
    return false;
  }

  const auto address = GetProcAddress(steamdll, "SteamAPI_ISteamUserStats_GetAchievement");
  if (!address) {
    fmt::println("Not GetProcAddress SteamAPI_ISteamUserStats_GetAchievement");
    return false;
  }

  using GetAchievement_t = bool(S_CALLTYPE *)(void*, const char*, bool*);
  bool achieved = false;
  return reinterpret_cast<GetAchievement_t>(address)(stats, name, &achieved) && achieved;
}

bool SetAchievement(const char* name) {
  const auto stats = SteamUserStats();
  if (!stats) {
    return false;
  }

  const auto address = GetProcAddress(steamdll, "SteamAPI_ISteamUserStats_SetAchievement");
  if (!address) {
    return false;
  }

  using SetAchievement_t = bool(S_CALLTYPE *)(void*, const char*);
  return reinterpret_cast<SetAchievement_t>(address)(stats, name);
}

bool StoreStats() {
  const auto stats = SteamUserStats();
  if (!stats) {
    return false;
  }

  const auto address = GetProcAddress(steamdll, "SteamAPI_ISteamUserStats_StoreStats");
  if (!address) {
    return false;
  }

  using StoreStats_t = bool(S_CALLTYPE *)(void*);
  return reinterpret_cast<StoreStats_t>(address)(stats);
}

#else

bool SteamAPI_InitSafe() { return false; }
void SteamAPI_Shutdown() {}
void SteamAPI_RunCallbacks() {}
void* SteamUserStats() { return nullptr; }
bool GetAchievement(const char*) { return false; }
bool SetAchievement(const char*) { return false; }
bool StoreStats() { return false; }

#endif
