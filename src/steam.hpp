#pragma once

#include "common.hpp"

bool SteamAPI_Init();
void SteamAPI_Shutdown();
void SteamAPI_RunCallbacks();
void* SteamUserStats();
bool GetAchievement(const char* name);
bool SetAchievement(const char* name);
bool StoreStats();
