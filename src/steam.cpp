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

using SteamAPI_InitSafe_t     = bool(S_CALLTYPE*)();
using SteamAPI_Shutdown_t     = void(S_CALLTYPE*)();
using SteamAPI_RunCallbacks_t = void(S_CALLTYPE*)();
using SteamUserStats_t        = void*(S_CALLTYPE*)();
using GetAchievement_t        = bool(S_CALLTYPE*)(void*, const char*, bool*);
using SetAchievement_t        = bool(S_CALLTYPE*)(void*, const char*);
using StoreStats_t            = bool(S_CALLTYPE*)(void*);
using SteamFriends_t          = void*(S_CALLTYPE*)();
using GetPersonaName_t        = const char*(S_CALLTYPE*)(void*);
using GetFriendCount_t        = int(S_CALLTYPE*)(void*, int);
using GetFriendByIndex_t      = uint64_t(S_CALLTYPE*)(void*, int, int);
using GetFriendPersonaName_t  = const char*(S_CALLTYPE*)(void*, uint64_t);

static DYNLIB_HANDLE hSteamApi = DYNLIB_LOAD(STEAM_LIB_NAME);

#define LOAD_SYMBOL(name, sym) reinterpret_cast<name>(reinterpret_cast<void*>(DYNLIB_SYM(hSteamApi, sym)))

static const auto pSteamAPI_InitSafe     = LOAD_SYMBOL(SteamAPI_InitSafe_t, "SteamAPI_InitSafe");
static const auto pSteamAPI_Shutdown     = LOAD_SYMBOL(SteamAPI_Shutdown_t, "SteamAPI_Shutdown");
static const auto pSteamAPI_RunCallbacks = LOAD_SYMBOL(SteamAPI_RunCallbacks_t, "SteamAPI_RunCallbacks");
static const auto pSteamUserStats        = LOAD_SYMBOL(SteamUserStats_t, "SteamAPI_SteamUserStats_v013");
static const auto pGetAchievement        = LOAD_SYMBOL(GetAchievement_t, "SteamAPI_ISteamUserStats_GetAchievement");
static const auto pSetAchievement        = LOAD_SYMBOL(SetAchievement_t, "SteamAPI_ISteamUserStats_SetAchievement");
static const auto pStoreStats            = LOAD_SYMBOL(StoreStats_t, "SteamAPI_ISteamUserStats_StoreStats");
static const auto pSteamFriends          = LOAD_SYMBOL(SteamFriends_t, "SteamAPI_SteamFriends_v017");
static const auto pGetPersonaName        = LOAD_SYMBOL(GetPersonaName_t, "SteamAPI_ISteamFriends_GetPersonaName");
static const auto pGetFriendCount        = LOAD_SYMBOL(GetFriendCount_t, "SteamAPI_ISteamFriends_GetFriendCount");
static const auto pGetFriendByIndex      = LOAD_SYMBOL(GetFriendByIndex_t, "SteamAPI_ISteamFriends_GetFriendByIndex");
static const auto pGetFriendPersonaName  = LOAD_SYMBOL(GetFriendPersonaName_t, "SteamAPI_ISteamFriends_GetFriendPersonaName");

bool SteamAPI_InitSafe() noexcept {
  if (pSteamAPI_InitSafe) {
    return pSteamAPI_InitSafe();
  }

  return false;
}

void SteamAPI_Shutdown() noexcept {
  if (pSteamAPI_Shutdown) {
    pSteamAPI_Shutdown();
  }
}

void SteamAPI_RunCallbacks() noexcept {
  if (pSteamAPI_RunCallbacks) {
    pSteamAPI_RunCallbacks();
  }
}

void* SteamUserStats() noexcept {
  if (pSteamUserStats) {
    return pSteamUserStats();
  }

  return nullptr;
}

void* SteamFriends() noexcept {
  if (pSteamFriends) {
    return pSteamFriends();
  }

  return nullptr;
}

bool GetAchievement(const char* name) noexcept {
  if (pGetAchievement) {
    bool achieved = false;
    return pGetAchievement(SteamUserStats(), name, &achieved) && achieved;
  }

  return false;
}

bool SetAchievement(const char* name) noexcept {
  if (pSetAchievement) {
    return pSetAchievement(SteamUserStats(), name);
  }

  return false;
}

bool StoreStats() noexcept {
  if (pStoreStats) {
    return pStoreStats(SteamUserStats());
  }

  return false;
}

std::string GetPersonaName() noexcept {
  if (pGetPersonaName) {
    if (const char* name = pGetPersonaName(SteamFriends())) {
      return {name};
    }
  }

  return {};
}

int GetFriendCount() noexcept {
  if (pGetFriendCount) {
    // k_EFriendFlagImmediate = 0x04
    return pGetFriendCount(SteamFriends(), 0x04);
  }

  return 0;
}

uint64_t GetFriendByIndex(int index) noexcept {
  if (pGetFriendByIndex) {
    return pGetFriendByIndex(SteamFriends(), index, 0x04);
  }

  return 0;
}

std::string GetFriendPersonaName(uint64_t steamId) noexcept {
  if (pGetFriendPersonaName) {
    if (const char* name = pGetFriendPersonaName(SteamFriends(), steamId)) {
      return {name};
    }
  }

  return {};
}

#else

bool SteamAPI_InitSafe() noexcept                   { return false; }
void SteamAPI_Shutdown() noexcept                   {}
void SteamAPI_RunCallbacks() noexcept               {}
void* SteamUserStats() noexcept                     { return nullptr; }
void* SteamFriends() noexcept                       { return nullptr; }
bool GetAchievement(const char*) noexcept           { return false; }
bool SetAchievement(const char*) noexcept           { return false; }
bool StoreStats() noexcept                          { return false; }
std::string GetPersonaName() noexcept               { return {}; }
int GetFriendCount() noexcept                       { return 0; }
uint64_t GetFriendByIndex(int) noexcept             { return 0; }
std::string GetFriendPersonaName(uint64_t) noexcept { return {}; }

#endif
