#pragma once

#include "common.hpp"

namespace memory {
class observable final {
public:
  sol::object value() const;
  void set(const sol::object& value);
  void subscribe(const sol::function& callback);

private:
  sol::object _value;
  std::vector<sol::function> _subscribers;
};

class kv final {
public:
  std::shared_ptr<memory::observable> get(const std::string& key, const sol::object& default_value = sol::lua_nil);
  void set(const std::string& key, const sol::object& value);
  // void subscribe(const std::string& key, const sol::function& callback, sol::this_state state);

private:
  std::unordered_map<std::string, std::shared_ptr<observable>> _values;
};
}
