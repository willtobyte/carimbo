#pragma once

#include "common.hpp"

#include <variant>

class cassette final {
public:
  using value_type = std::variant<
    std::nullptr_t,
    bool,
    int64_t,
    uint64_t,
    double,
    std::string
  >;

  cassette();
  ~cassette() noexcept = default;

  template <typename T>
  void set(std::string_view key, const T& value);

  template <typename T>
  T get(std::string_view key, const T& fallback) const;

  void clear(std::string_view key) noexcept;

  const value_type* find(std::string_view key) const noexcept;

private:
  boost::unordered_flat_map<std::string, value_type, transparent_string_hash, std::equal_to<>> _data;

#ifndef EMSCRIPTEN
  static constexpr const char* _filename = "cassette.tape";
#else
  static constexpr const char* _storagekey = "cassette";
#endif

  void persist() const noexcept;
};

#include "cassette.tpp"
