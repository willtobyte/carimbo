#pragma once

#include "common.hpp"

bool SteamAPI_InitSafe() noexcept;
void SteamAPI_Shutdown() noexcept;
void SteamAPI_RunCallbacks() noexcept;
void* SteamUserStats() noexcept;
bool GetAchievement(const char* name) noexcept;
bool SetAchievement(const char* name) noexcept;
bool StoreStats() noexcept;
void* SteamFriends() noexcept;
std::string GetPersonaName() noexcept;
int GetFriendCount() noexcept;
uint64_t GetFriendByIndex(int index) noexcept;
std::string GetFriendPersonaName(uint64_t steamId) noexcept;
