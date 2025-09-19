#include "kv.hpp"

using namespace memory;

sol::object observable::get() const {
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

sol::object kv::get(const std::string& key, sol::this_state state) {
  return ensure(key, state.L)->get();
}

void kv::set(const std::string& key, const sol::object& value, sol::this_state state) {
  ensure(key, state.L)->set(value);
}

void kv::subscribe(const std::string& key, const sol::function& callback, sol::this_state state) {
  ensure(key, state.L)->subscribe(callback);
}

void kv::unset(const std::string& key, sol::this_state state) {
  if (auto it = _values.find(key); it != _values.end()) {
    it->second->set(sol::make_object(state.L, sol::lua_nil));
    _values.erase(it);
  }
}

void kv::incr(const std::string& key, sol::this_state s) {
  add(key, 1, s);
}

void kv::incrby(const std::string& key, int64_t value, sol::this_state s) {
  add(key, value, s);
}

void kv::decr(const std::string& key, sol::this_state s) {
  add(key, -1, s);
}

void kv::decrby(const std::string& key, int64_t value, sol::this_state s) {
  add(key, -value, s);
}

void kv::add(const std::string& key, int64_t delta, sol::this_state state) {
  auto* L = state.L;
  const auto o = ensure(key, L);
  const auto value = o->get();

  if (!value.valid()) {
    o->set(sol::make_object(L, delta));
    return;
  }

  if (value.get_type() == sol::type::lua_nil) {
    o->set(sol::make_object(L, delta));
    return;
  }

  if (value.is<double>()) {
    const auto v = value.as<double>() + static_cast<double>(delta);
    o->set(sol::make_object(L, v));
    return;
  }

  if (value.is<int64_t>()) {
    const auto v = value.as<int64_t>() + delta;
    o->set(sol::make_object(L, v));
    return;
  }

  if (value.is<int>()) {
    const auto v = static_cast<int64_t>(value.as<int>()) + delta;
    o->set(sol::make_object(L, v));
    return;
  }
}

std::shared_ptr<observable> kv::ensure(const std::string& key, lua_State *L) {
  const auto [it, inserted] = _values.try_emplace(key, std::make_shared<observable>());
  if (inserted) {
    it->second->set(sol::make_object(L, sol::lua_nil));
  }

  return it->second;
}
