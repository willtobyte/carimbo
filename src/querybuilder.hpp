#pragma once

#include "common.hpp"

namespace network {
class querybuilder final {
public:
  querybuilder& add(std::string_view key, std::string_view value);

  [[nodiscard]] std::string build() const;

  static std::string make(std::initializer_list<std::pair<std::string, std::string>> entries);

private:
  std::unordered_map<std::string, std::string> _parameters;
};
}
