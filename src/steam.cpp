#include "steam.hpp"

#ifdef _WIN32
#include <windows.h>

#ifndef S_CALLTYPE
#define S_CALLTYPE __cdecl
#endif

static const HMODULE steamdll = LoadLibraryA("steam_api64.dll");

bool SteamAPI_Init() {
  if (!steamdll) {
    return false;
  }

  const auto address = GetProcAddress(steamdll, "SteamAPI_Init");
  if (!address) {
    return false;
  }

  using SteamAPI_Init_t = bool(S_CALLTYPE *)();
  return reinterpret_cast<SteamAPI_Init_t>(address)();
}

#else

bool SteamAPI_Init() {
  return false;
}

#endif
