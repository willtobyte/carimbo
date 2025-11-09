#include "kv.hpp"

using namespace memory;

sol::object observable::value() const {
  return _value;
}

void observable::set(const sol::object& value) {
  _value = value;

  if (const auto& fn = _subscriber; fn) {
    fn(value);
  }
}

void observable::subscribe(sol::protected_function callback) {
  _subscriber = interop::wrap_fn<const sol::object&>(std::move(callback));
}

void observable::unsubscribe() {
  _subscriber = nullptr;
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
