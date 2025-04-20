#pragma once

template<typename T>
void cassette::set(const std::string &key, const T &value) {
  if (key.empty()) {
    return;
  }

  _j[key] = value;
  persist();
}

template<typename T>
T cassette::get(const std::string &key, const T &default_value) const {
  if (!_j.contains(key)) {
    return default_value;
  }

  const auto &entry = _j.at(key);

  if (entry.is_discarded()) {
    panic("JSON entry for key '{}' is discarded", key);
  }

  if constexpr (std::is_same_v<T, int>) {
    if (!entry.is_number_integer())
      panic("Type mismatch for key '{}': expected int, got {}", key, entry.type_name());
  } else if constexpr (std::is_same_v<T, double>) {
    if (!entry.is_number())
      panic("Type mismatch for key '{}': expected double, got {}", key, entry.type_name());
  } else if constexpr (std::is_same_v<T, bool>) {
    if (!entry.is_boolean())
      panic("Type mismatch for key '{}': expected bool, got {}", key, entry.type_name());
  } else if constexpr (std::is_same_v<T, std::string>) {
    if (!entry.is_string())
      panic("Type mismatch for key '{}': expected string, got {}", key, entry.type_name());
  } else {
    static_assert(sizeof(T) == 0, "cassette::get does not support this type");
  }

  return entry.get<T>();
}
