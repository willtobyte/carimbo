#pragma once

#include "common.hpp"

namespace storage {
class cassette final {
public:
  cassette();
  ~cassette() = default;

  cassette(const cassette&) = default;
  cassette& operator=(const cassette&) = default;
  cassette(cassette&&) noexcept = default;
  cassette& operator=(cassette&&) noexcept = default;

  template <typename T>
  void set(const std::string& key, const T& value);

  template <typename T>
  T get(const std::string& key, const T& default_value) const;

  void clear(const std::string& key) noexcept;

private:
  nlohmann::json _j;
#ifndef EMSCRIPTEN
  static constexpr const char* _filename = "cassette.tape";
#else
  static constexpr const char* _cookiekey = "cassette=";
#endif

  void persist() const noexcept;
};

#include "cassette.tpp"
}
