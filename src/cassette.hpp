#pragma once

#include "common.hpp"

class cassette final {
public:
  cassette();
  ~cassette() = default;

  template <typename T>
  void set(std::string_view key, const T& value);

  template <typename T>
  T get(std::string_view key, const T& default_value) const;

  void clear(std::string_view key) noexcept;

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
