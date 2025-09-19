#pragma once

#include "common.hpp"

namespace memory {
class observable final {
public:
  sol::object get() const;
  void set(const sol::object& value);
  void subscribe(const sol::function& callback);

private:
  sol::object _value;
  std::vector<sol::function> _subscribers;
};

class kv final {
public:
  sol::object get(const std::string& key, sol::this_state state, const sol::object& default_value = sol::lua_nil);
  void set(const std::string& key, const sol::object& value);
  void subscribe(const std::string& key, const sol::function& callback, sol::this_state state);
  void unset(const std::string& key, sol::this_state state);
  void incr(const std::string& key, sol::this_state state);
  void incrby(const std::string& key, int64_t value, sol::this_state state);
  void decr(const std::string& key, sol::this_state state);
  void decrby(const std::string& key, int64_t value, sol::this_state state);
  sol::object getset(const std::string& key, const sol::object& value, sol::this_state state);
  bool setnx(const std::string& key, const sol::object& value);

private:
  std::unordered_map<std::string, std::shared_ptr<observable>> _values;
};
}
