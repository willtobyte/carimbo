#pragma once

template<typename T>
void cassete::set(const std::string &key, const T &value) {
  if (key.empty()) {
    return;
  }

  _j[key] = value;
  persist();
}

template<typename T>
std::optional<T> cassete::get(const std::string &key) const {
  if (!_j.contains(key)) {
    return std::nullopt;
  }

  try {
    return _j.at(key).get<T>();
  } catch (const nlohmann::json::exception &e) {
    return std::nullopt;
  }
}
