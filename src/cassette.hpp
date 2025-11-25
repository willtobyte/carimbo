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

template<typename T>
void cassette::set(std::string_view key, const T& value) {
  if (key.empty()) [[unlikely]] {
    return;
  }

  _j[key] = value;
  persist();
}

template<typename T>
T cassette::get(std::string_view key, const T& default_value) const {
  if (!_j.contains(key)) {
    return default_value;
  }

  const auto& entry = _j.at(key);

  return entry.template get<T>();
}
