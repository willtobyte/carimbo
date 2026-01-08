#pragma once

#include "common.hpp"

namespace network {
class querybuilder final {
public:
  querybuilder& add(std::string_view key, std::string_view value);

  [[nodiscard]] std::string build() const;

  static std::string make(std::initializer_list<std::pair<std::string_view, std::string_view>> entries);

private:
  boost::unordered_flat_map<std::string, std::string, transparent_string_hash, std::equal_to<>> _parameters;
};
}
