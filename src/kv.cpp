#include "kv.hpp"

sol::object observable::value() const {
  return _value;
}

void observable::set(const sol::object& value) {
  _value = value;
  for (const auto& subscriber : _subscribers) {
    subscriber(value);
  }
}

uint32_t observable::subscribe(sol::protected_function callback) {
  const auto id = static_cast<uint32_t>(_subscribers.size());
  _subscribers.emplace_back(std::move(callback));
  return id;
}

void observable::unsubscribe(uint32_t id) {
  if (id < _subscribers.size()) {
    _subscribers[id] = nullptr;
  }
}

std::shared_ptr<observable> kv::get(std::string_view key, const sol::object& fallback) {
  auto [it, inserted] = _values.try_emplace(key);

  if (inserted) {
    it->second = std::make_shared<observable>();
    it->second->set(fallback);
  }

  return it->second;
}

void kv::set(std::string_view key, const sol::object& value) {
  const auto [it, inserted] = _values.try_emplace(key);

  if (inserted) {
    it->second = std::make_shared<observable>();
  }

  it->second->set(value);
}
