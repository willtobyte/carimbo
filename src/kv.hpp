#pragma once

#include "common.hpp"

class observable final {
public:
  ~observable() = default;
  sol::object value() const;
  void set(const sol::object& value);
  void subscribe(sol::protected_function callback);
  void unsubscribe();

private:
  functor _subscriber;
  sol::object _value;
};

class kv final {
public:
  ~kv() = default;
  std::shared_ptr<observable> get(std::string_view key, const sol::object& default_value = sol::lua_nil);
  void set(std::string_view key, const sol::object& value);

private:
  std::unordered_map<std::string, std::shared_ptr<observable>, string_hash, string_equal> _values;
};