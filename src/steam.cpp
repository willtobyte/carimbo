#include "steam.hpp"

#if defined(_WIN32)
  #include <windows.h>
  #define DYNLIB_HANDLE HMODULE
  #define DYNLIB_LOAD(name) LoadLibraryA(name)
  #define DYNLIB_SYM(lib, name) GetProcAddress(lib, name)
  #define STEAM_LIB_NAME "steam_api64.dll"
#elif defined(__APPLE__)
  #include <dlfcn.h>
  #define DYNLIB_HANDLE void*
  #define DYNLIB_LOAD(name) dlopen(name, RTLD_LAZY)
  #define DYNLIB_SYM(lib, name) dlsym(lib, name)
  #define STEAM_LIB_NAME "libsteam_api.dylib"
#endif

#ifndef S_CALLTYPE
  #define S_CALLTYPE __cdecl
#endif

#if defined(DYNLIB_LOAD)

using SteamAPI_InitSafe_t     = bool(S_CALLTYPE *)();
using SteamAPI_Shutdown_t     = void(S_CALLTYPE *)();
using SteamAPI_RunCallbacks_t = void(S_CALLTYPE *)();
using SteamUserStats_t        = void*(S_CALLTYPE *)();
using GetAchievement_t        = bool(S_CALLTYPE *)(void*, const char*, bool*);
using SetAchievement_t        = bool(S_CALLTYPE *)(void*, const char*);
using StoreStats_t            = bool(S_CALLTYPE *)(void*);

static DYNLIB_HANDLE hSteamApi = DYNLIB_LOAD(STEAM_LIB_NAME);

#define LOAD_SYMBOL(name, sym) reinterpret_cast<name>(reinterpret_cast<void*>(DYNLIB_SYM(hSteamApi, sym)))

static const auto pSteamAPI_InitSafe     = LOAD_SYMBOL(SteamAPI_InitSafe_t, "SteamAPI_InitSafe");
static const auto pSteamAPI_Shutdown     = LOAD_SYMBOL(SteamAPI_Shutdown_t, "SteamAPI_Shutdown");
static const auto pSteamAPI_RunCallbacks = LOAD_SYMBOL(SteamAPI_RunCallbacks_t, "SteamAPI_RunCallbacks");
static const auto pSteamUserStats        = LOAD_SYMBOL(SteamUserStats_t, "SteamAPI_SteamUserStats_v013");
static const auto pGetAchievement        = LOAD_SYMBOL(GetAchievement_t, "SteamAPI_ISteamUserStats_GetAchievement");
static const auto pSetAchievement        = LOAD_SYMBOL(SetAchievement_t, "SteamAPI_ISteamUserStats_SetAchievement");
static const auto pStoreStats            = LOAD_SYMBOL(StoreStats_t, "SteamAPI_ISteamUserStats_StoreStats");

bool SteamAPI_InitSafe() {
  if (pSteamAPI_InitSafe) {
    return pSteamAPI_InitSafe();
  }

  return false;
}

void SteamAPI_Shutdown() {
  if (pSteamAPI_Shutdown) {
    pSteamAPI_Shutdown();
  }
}

void SteamAPI_RunCallbacks() {
  if (pSteamAPI_RunCallbacks) {
    pSteamAPI_RunCallbacks();
  }
}

void* SteamUserStats() {
  if (pSteamUserStats) {
    return pSteamUserStats();
  }

  return nullptr;
}

bool GetAchievement(const char* name) {
  if (pGetAchievement) {
    bool achieved = false;
    return pGetAchievement(SteamUserStats(), name, &achieved) && achieved;
  }

  return false;
}

bool SetAchievement(const char* name) {
  if (pSetAchievement) {
    return pSetAchievement(SteamUserStats(), name);
  }

  return false;
}

bool StoreStats() {
  if (pStoreStats) {
    pStoreStats(SteamUserStats());
  }

  return false;
}

#else

bool SteamAPI_InitSafe()            { return false; }
void SteamAPI_Shutdown()           {}
void SteamAPI_RunCallbacks()       {}
void* SteamUserStats()             { return nullptr; }
bool GetAchievement(const char*)   { return false; }
bool SetAchievement(const char*)   { return false; }
bool StoreStats()                  { return false; }

#endif
