#include "kv.hpp"

using namespace memory;

sol::object observable::value() const {
  return _value;
}

void observable::set(const sol::object& value) {
  _value = value;
  for (const auto& callback : _subscribers) {
    callback(value);
  }
}

void observable::subscribe(const sol::function& callback) {
  _subscribers.emplace_back(callback);
}

std::shared_ptr<memory::observable> kv::get(const std::string& key, const sol::object& default_value) {
  auto [it, inserted] = _values.try_emplace(key, std::make_shared<observable>());

  if (inserted) {
    it->second->set(default_value);
  }

  return it->second;
}

void kv::set(const std::string& key, const sol::object& value) {
  const auto it = _values.try_emplace(key, std::make_shared<observable>()).first;

  it->second->set(value);
}
