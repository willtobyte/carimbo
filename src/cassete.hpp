#pragma once

#include "common.hpp"

namespace storage {
class cassete {
public:
  cassete();

  template <typename T>
  void set(const std::string &key, const T &value);

  template <typename T>
  T get(const std::string &key, const T &default_value) const;

  void clear(const std::string &key);

private:
  nlohmann::json _j;
#ifndef EMSCRIPTEN
  static constexpr const char *_filename = "cassete.json";
#else
  static constexpr const char *_cookiekey = "cassete=";
#endif

  void persist() const;
};

#include "cassete.tpp"
}
