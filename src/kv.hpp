#pragma once

#include "common.hpp"

namespace memory {
class observable final {
public:
  sol::object get() const;
  void set(const sol::object& new_value);
  void subscribe(const sol::function& callback);

private:
  sol::object _value;
  std::vector<sol::function> _subscribers;
};

class kv final {
public:
  sol::object get(const std::string& key, sol::this_state state);
  void set(const std::string& key, const sol::object& new_value, sol::this_state state);
  void subscribe(const std::string& key, const sol::function& callback, sol::this_state state);

protected:
  std::shared_ptr<observable> ensure(const std::string& key, lua_State *L);

private:
  std::unordered_map<std::string, std::shared_ptr<observable>> _values;
};
}
