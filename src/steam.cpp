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

static DYNLIB_HANDLE _steam_handle() {
  static DYNLIB_HANDLE h = DYNLIB_LOAD(STEAM_LIB_NAME);
  return h;
}

template<typename T>
static T _load_symbol(const char* name) {
  return reinterpret_cast<T>(reinterpret_cast<void*>(DYNLIB_SYM(_steam_handle(), name)));
}

static SteamAPI_InitSafe_t _SteamAPI_InitSafe() {
  static auto p = _load_symbol<SteamAPI_InitSafe_t>("SteamAPI_InitSafe");
  return p;
}

static SteamAPI_Shutdown_t _SteamAPI_Shutdown() {
  static auto p = _load_symbol<SteamAPI_Shutdown_t>("SteamAPI_Shutdown");
  return p;
}

static SteamAPI_RunCallbacks_t _SteamAPI_RunCallbacks() {
  static auto p = _load_symbol<SteamAPI_RunCallbacks_t>("SteamAPI_RunCallbacks");
  return p;
}

static SteamUserStats_t _SteamUserStats() {
  static auto p = _load_symbol<SteamUserStats_t>("SteamAPI_SteamUserStats_v013");
  return p;
}

static GetAchievement_t _GetAchievement() {
  static auto p = _load_symbol<GetAchievement_t>("SteamAPI_ISteamUserStats_GetAchievement");
  return p;
}

static SetAchievement_t _SetAchievement() {
  static auto p = _load_symbol<SetAchievement_t>("SteamAPI_ISteamUserStats_SetAchievement");
  return p;
}

static StoreStats_t _StoreStats() {
  static auto p = _load_symbol<StoreStats_t>("SteamAPI_ISteamUserStats_StoreStats");
  return p;
}

bool SteamAPI_InitSafe() {
  const auto fn = _SteamAPI_InitSafe();
  if (fn) [[likely]] {
    return fn();
  }

  return false;
}

void SteamAPI_Shutdown() {
  const auto fn = _SteamAPI_Shutdown();
  if (fn) [[likely]] {
    fn();
  }
}

void SteamAPI_RunCallbacks() {
  const auto fn = _SteamAPI_RunCallbacks();
  if (fn) [[likely]] {
    fn();
  }
}

void* SteamUserStats() {
  const auto fn = _SteamUserStats();
  if (fn) [[likely]] {
    return fn();
  }

  return nullptr;
}

bool GetAchievement(const char* name) {
  const auto fn = _GetAchievement();
  if (fn) [[likely]] {
    bool achieved = false;
    return fn(SteamUserStats(), name, &achieved) && achieved;
  }

  return false;
}

bool SetAchievement(const char* name) {
  const auto fn = _SetAchievement();
  if (fn) [[likely]] {
    return fn(SteamUserStats(), name);
  }

  return false;
}

bool StoreStats() {
  const auto fn = _StoreStats();
  if (fn) [[likely]] {
    return fn(SteamUserStats());
  }

  return false;
}

#else

bool SteamAPI_InitSafe()           { return false; }
void SteamAPI_Shutdown()           {}
void SteamAPI_RunCallbacks()       {}
void* SteamUserStats()             { return nullptr; }
bool GetAchievement(const char*)   { return false; }
bool SetAchievement(const char*)   { return false; }
bool StoreStats()                  { return false; }

#endif
