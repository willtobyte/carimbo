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
  if (key.empty()) {
    return std::nullopt;
  }

  if (auto it = _j.find(key); it != _j.end()) {
    if (auto ptr = it->get_ptr<const T*>()) {
      return *ptr;
    }
  }

  return std::nullopt;
}
