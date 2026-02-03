#include "user.hpp"
#include "steam.hpp"

buddy::buddy(uint64_t id, std::string name) noexcept
    : _id(id), _name(std::move(name)) {}

uint64_t buddy::id() const noexcept { return _id; }

const std::string& buddy::name() const noexcept { return _name; }

std::string user::persona() const noexcept {
  if (!SteamFriends()) [[unlikely]] {
    return {};
  }

  return GetPersonaName();
}

std::vector<buddy> user::buddies() const noexcept {
  if (!SteamFriends()) [[unlikely]] {
    return {};
  }

  std::vector<buddy> result;
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
