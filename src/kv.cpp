#include "kv.hpp"

observable::observable() {
  _subscribers.reserve(8);
}

sol::object observable::value() const noexcept {
  return _value;
}

void observable::set(const sol::object& value) {
  _value = value;

  for (const auto& [_, subscriber] : _subscribers) {
    subscriber(value);
  }
}

uint32_t observable::subscribe(sol::protected_function callback) {
  const auto id = _next_id++;
  _subscribers.emplace(id, std::move(callback));
  return id;
}

void observable::unsubscribe(uint32_t id) noexcept {
  _subscribers.erase(id);
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
