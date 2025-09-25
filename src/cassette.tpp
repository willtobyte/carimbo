#pragma once

template<typename T>
void cassette::set(const std::string& key, const T& value) {
  if (key.empty()) [[unlikely]] {
    return;
  }

  _j[key] = value;
  persist();
}

template<typename T>
T cassette::get(const std::string& key, const T& default_value) const {
  if (!_j.contains(key)) {
    return default_value;
  }

  const auto& entry = _j.at(key);

  return entry.get<T>();
}
