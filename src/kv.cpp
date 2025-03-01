#include "kv.hpp"

using namespace memory;
sol::object observable::get() const noexcept {
  return _value;
}

void observable::set(const sol::object &new_value) {
  _value = new_value;
  for (auto &callback : _subscribers) {
    callback(new_value);
  }
}

void observable::subscribe(const sol::function &callback) {
  _subscribers.emplace_back(callback);
}

sol::object kv::get(const std::string &key, sol::this_state state) {
  return ensure_observable(key, state.L)->get();
}

void kv::set(const std::string &key, const sol::object &new_value, sol::this_state state) {
  ensure_observable(key, state.L)->set(new_value);
}

void kv::subscribe(const std::string &key, const sol::function &callback, sol::this_state state) {
  ensure_observable(key, state.L)->subscribe(callback);
}

std::shared_ptr<observable> kv::ensure_observable(const std::string &key, lua_State *L) {
  auto [it, inserted] = _values.try_emplace(key, std::make_shared<observable>());
  if (inserted) {
    it->second->set(sol::make_object(L, sol::lua_nil));
  }

  return it->second;
}
