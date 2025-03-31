#pragma once

#include "common.hpp"

namespace storage {
class cassette {
public:
  cassette();

  template <typename T>
  void set(const std::string &key, const T &value);

  template <typename T>
  T get(const std::string &key, const T &default_value) const;

  void clear(const std::string &key);

private:
  nlohmann::json _j;
#ifndef EMSCRIPTEN
  static constexpr const char *_filename = "cassette.json";
#else
  static constexpr const char *_cookiekey = "cassette=";
#endif

  void persist() const;
};

#include "cassette.tpp"
}
