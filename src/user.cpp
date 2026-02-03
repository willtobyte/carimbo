#include "user.hpp"
#include "steam.hpp"

std::string user::persona() const noexcept {
  if (!SteamFriends()) [[unlikely]] {
    return {};
  }

  return GetPersonaName();
}

std::vector<std::pair<uint64_t, std::string>> user::friends() const noexcept {
  if (!SteamFriends()) [[unlikely]] {
    return {};
  }

  std::vector<std::pair<uint64_t, std::string>> result;
  const auto count = GetFriendCount();

  result.reserve(static_cast<size_t>(count));

  for (int i = 0; i < count; ++i) {
    if (const auto id = GetFriendByIndex(i)) {
      if (auto name = GetFriendPersonaName(id); !name.empty()) {
        result.emplace_back(id, std::move(name));
      }
    }
  }

  return result;
}
