#include "steam.hpp"

#ifdef _WIN32
#include <windows.h>

#ifndef S_CALLTYPE
#define S_CALLTYPE __cdecl
#endif

static const HMODULE hSteamApi = LoadLibraryA("steam_api64.dll");

using SteamAPI_InitSafe_t = bool(S_CALLTYPE *)();
using SteamAPI_Shutdown_t = void(S_CALLTYPE *)();
using SteamAPI_RunCallbacks_t = void(S_CALLTYPE *)();
using SteamUserStats_t = void*(S_CALLTYPE *)();
using GetAchievement_t = bool(S_CALLTYPE *)(void*, const char*, bool*);
using SetAchievement_t = bool(S_CALLTYPE *)(void*, const char*);
using StoreStats_t = bool(S_CALLTYPE *)(void*);

static const SteamAPI_InitSafe_t pSteamAPI_InitSafe =
  reinterpret_cast<SteamAPI_InitSafe_t>(GetProcAddress(hSteamApi, "SteamAPI_InitSafe"));

static const SteamAPI_Shutdown_t pSteamAPI_Shutdown =
  reinterpret_cast<SteamAPI_Shutdown_t>(GetProcAddress(hSteamApi, "SteamAPI_Shutdown"));

static const SteamAPI_RunCallbacks_t pSteamAPI_RunCallbacks =
  reinterpret_cast<SteamAPI_RunCallbacks_t>(GetProcAddress(hSteamApi, "SteamAPI_RunCallbacks"));

static const SteamUserStats_t pSteamUserStats =
  reinterpret_cast<SteamUserStats_t>(GetProcAddress(hSteamApi, "SteamAPI_SteamUserStats_v013"));

static const GetAchievement_t pGetAchievement =
  reinterpret_cast<GetAchievement_t>(GetProcAddress(hSteamApi, "SteamAPI_ISteamUserStats_GetAchievement"));

static const SetAchievement_t pSetAchievement =
  reinterpret_cast<SetAchievement_t>(GetProcAddress(hSteamApi, "SteamAPI_ISteamUserStats_SetAchievement"));

static const StoreStats_t pStoreStats =
  reinterpret_cast<StoreStats_t>(GetProcAddress(hSteamApi, "SteamAPI_ISteamUserStats_StoreStats"));

bool SteamAPI_InitSafe() {
  return pSteamAPI_InitSafe();
}

void SteamAPI_Shutdown() {
  pSteamAPI_Shutdown();
}

void SteamAPI_RunCallbacks() {
  pSteamAPI_RunCallbacks();
}

void* SteamUserStats() {
  return pSteamUserStats();
}

bool GetAchievement(const char* name) {
  bool achieved = false;
  return pGetAchievement(SteamUserStats(), name, &achieved) && achieved;
}

bool SetAchievement(const char* name) {
  return pSetAchievement(SteamUserStats(), name);
}

bool StoreStats() {
  return pStoreStats(SteamUserStats());
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
