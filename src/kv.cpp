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
  _subscriber = interop::wrap_fn<void(const sol::object&)>(std::move(callback));
}

void observable::unsubscribe() {
  _subscriber = nullptr;
}

std::shared_ptr<memory::observable> kv::get(std::string_view key, const sol::object& default_value) {
  auto [it, inserted] = _values.try_emplace(std::string{key});

  if (inserted) [[likely]] {
    it->second = std::make_shared<observable>();
    it->second->set(default_value);
  }

  return it->second;
}

void kv::set(std::string_view key, const sol::object& value) {
  const auto [it, inserted] = _values.try_emplace(std::string{key});

  if (inserted) [[likely]] {
    it->second = std::make_shared<observable>();
  }

  it->second->set(value);
}
