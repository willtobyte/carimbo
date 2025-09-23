#include "kv.hpp"

using namespace memory;

namespace {
static void add(
  const std::string& key,
  int64_t delta,
  std::unordered_map<std::string, std::shared_ptr<observable>>& values,
  sol::this_state state
) {
  auto* L = state.L;

  auto [it, inserted] = values.try_emplace(key, std::make_shared<observable>());
  if (inserted) {
    it->second->set(sol::make_object(L, delta));
    return;
  }

  const auto value = it->second->value();

  if (!value.valid()) {
    it->second->set(sol::make_object(L, delta));
    return;
  }

  if (value.get_type() == sol::type::lua_nil) {
    it->second->set(sol::make_object(L, delta));
    return;
  }

  if (value.is<double>()) {
    const auto v = value.as<double>() + static_cast<double>(delta);
    it->second->set(sol::make_object(L, v));
    return;
  }

  if (value.is<int64_t>()) {
    const auto v = value.as<int64_t>() + delta;
    it->second->set(sol::make_object(L, v));
    return;
  }

  if (value.is<int>()) {
    const auto v = static_cast<int64_t>(value.as<int>()) + delta;
    it->second->set(sol::make_object(L, v));
    return;
  }
}
}

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

void kv::subscribe(const std::string& key, const sol::function& callback, sol::this_state state) {
  const auto [it, inserted] = _values.try_emplace(key, std::make_shared<observable>());
  if (inserted) {
    it->second->set(sol::make_object(state.L, sol::lua_nil));
  }

  it->second->subscribe(callback);
}

void kv::unset(const std::string& key, sol::this_state state) {
  if (const auto it = _values.find(key); it != _values.end()) {
    it->second->set(sol::make_object(state.L, sol::lua_nil));
    _values.erase(it);
  }
}

void kv::incr(const std::string& key, sol::this_state state) {
  add(key, 1, _values, state);
}

void kv::incrby(const std::string& key, int64_t value, sol::this_state state) {
  add(key, value, _values, state);
}

void kv::decr(const std::string& key, sol::this_state state) {
  add(key, -1, _values, state);
}

void kv::decrby(const std::string& key, int64_t value, sol::this_state state) {
  add(key, -value, _values, state);
}

sol::object kv::getset(const std::string& key, const sol::object& value, sol::this_state state) {
  const auto [it, inserted] = _values.try_emplace(key, std::make_shared<observable>());
  if (inserted) {
    auto old = sol::make_object(state.L, sol::lua_nil);
    it->second->set(value);
    return old;
  }

  const auto old = it->second->value();
  it->second->set(value);
  return old;
}

bool kv::setnx(const std::string& key, const sol::object& value) {
  auto [it, inserted] = _values.try_emplace(key, std::make_shared<observable>());
  if (inserted) {
    it->second->set(value);
    return true;
  }

  const auto current = it->second->value();
  if (!current.valid()) {
    it->second->set(value);
    return true;
  }

  if (current.get_type() == sol::type::lua_nil) {
    it->second->set(value);
    return true;
  }

  return false;
}
