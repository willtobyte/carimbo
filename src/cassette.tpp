#pragma once

template<typename T>
void cassette::set(std::string_view key, const T& value) {
  if (key.empty()) [[unlikely]] {
    return;
  }

  if constexpr (std::is_same_v<T, std::nullptr_t>) {
    _data.insert_or_assign(std::string(key), nullptr);
  } else if constexpr (std::is_same_v<T, bool>) {
    _data.insert_or_assign(std::string(key), value);
  } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
    _data.insert_or_assign(std::string(key), static_cast<int64_t>(value));
  } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
    _data.insert_or_assign(std::string(key), static_cast<uint64_t>(value));
  } else if constexpr (std::is_floating_point_v<T>) {
    _data.insert_or_assign(std::string(key), static_cast<double>(value));
  } else if constexpr (std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::string_view>) {
    _data.insert_or_assign(std::string(key), std::string(value));
  } else {
    static_assert(sizeof(T) == 0, "unsupported type for cassette::set");
  }

  persist();
}

template<typename T>
T cassette::get(std::string_view key, const T& default_value) const {
  const auto it = _data.find(key);
  if (it == _data.end()) {
    return default_value;
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

  return default_value;
}
