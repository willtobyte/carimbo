#pragma once

#include "common.hpp"

namespace network {
class querybuilder final {
public:
  querybuilder &add(const std::string &key, const std::string &value) noexcept;

  [[nodiscard]] std::string build() const noexcept;

  static std::string make(std::initializer_list<std::pair<std::string, std::string>> entries);

private:
  std::unordered_map<std::string, std::string> _parameters;
};
}
