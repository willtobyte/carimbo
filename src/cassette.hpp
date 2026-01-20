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

  template<typename T>
  void set(std::string_view key, const T& value) {
    if (key.empty()) [[unlikely]] {
      return;
    }

    if constexpr (std::is_same_v<T, std::nullptr_t>) {
      _data.insert_or_assign(key, nullptr);
    } else if constexpr (std::is_same_v<T, bool>) {
      _data.insert_or_assign(key, value);
    } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
      _data.insert_or_assign(key, static_cast<int64_t>(value));
    } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
      _data.insert_or_assign(key, static_cast<uint64_t>(value));
    } else if constexpr (std::is_floating_point_v<T>) {
      _data.insert_or_assign(key, static_cast<double>(value));
    } else if constexpr (std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::string_view>) {
      _data.insert_or_assign(key, std::string(value));
    } else {
      static_assert(sizeof(T) == 0, "unsupported type for cassette::set");
    }

    persist();
  }

  template<typename T>
  T get(std::string_view key, const T& fallback) const {
    const auto it = _data.find(key);
    if (it == _data.end()) {
      return fallback;
    }

    const auto& storage = it->second;

    if constexpr (std::is_same_v<T, bool>) {
      if (const auto* v = std::get_if<bool>(&storage)) {
        return *v;
      }
    } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
      if (const auto* v = std::get_if<int64_t>(&storage)) {
        return static_cast<T>(*v);
      }
      if (const auto* v = std::get_if<uint64_t>(&storage)) {
        return static_cast<T>(*v);
      }
      if (const auto* v = std::get_if<double>(&storage)) {
        return static_cast<T>(*v);
      }
    } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
      if (const auto* v = std::get_if<uint64_t>(&storage)) {
        return static_cast<T>(*v);
      }
      if (const auto* v = std::get_if<int64_t>(&storage)) {
        return static_cast<T>(*v);
      }
      if (const auto* v = std::get_if<double>(&storage)) {
        return static_cast<T>(*v);
      }
    } else if constexpr (std::is_floating_point_v<T>) {
      if (const auto* v = std::get_if<double>(&storage)) {
        return static_cast<T>(*v);
      }
      if (const auto* v = std::get_if<int64_t>(&storage)) {
        return static_cast<T>(*v);
      }
      if (const auto* v = std::get_if<uint64_t>(&storage)) {
        return static_cast<T>(*v);
      }
    } else if constexpr (std::is_same_v<T, std::string>) {
      if (const auto* v = std::get_if<std::string>(&storage)) {
        return *v;
      }
    } else if constexpr (std::is_same_v<T, std::string_view>) {
      if (const auto* v = std::get_if<std::string>(&storage)) {
        return std::string_view(*v);
      }
    }

    return fallback;
  }

  void clear(std::string_view key);
  void clear();

  std::optional<value_type> find(std::string_view key) const noexcept;

private:
  boost::unordered_flat_map<std::string, value_type, transparent_string_hash, std::equal_to<>> _data;

#ifndef EMSCRIPTEN
  static constexpr const char* _filename = "cassette.tape";
#else
  static constexpr const char* _storagekey = "cassette";
#endif

  void persist() const;
};
