#pragma once

#include "common.hpp"

namespace memory {
class observable final {
public:
  ~observable() noexcept = default;
  sol::object value() const;
  void set(const sol::object& value);
  void subscribe(const std::function<void(const sol::object&)>&& callback) noexcept;
  void unsubscribe() noexcept;

private:
  std::function<void(const sol::object&)> _subscriber;
  sol::object _value;
};

class kv final {
public:
  ~kv() noexcept = default;
  std::shared_ptr<memory::observable> get(const std::string& key, const sol::object& default_value = sol::lua_nil);
  void set(const std::string& key, const sol::object& value);

private:
  std::unordered_map<std::string, std::shared_ptr<observable>> _values;
};
}
