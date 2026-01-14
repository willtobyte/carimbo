#pragma once

#include "common.hpp"

class observable final {
public:
  observable();
  ~observable() noexcept = default;

  [[nodiscard]] sol::object value() const noexcept;
  void set(const sol::object& value);
  [[nodiscard]] uint32_t subscribe(sol::protected_function callback);
  void unsubscribe(uint32_t id) noexcept;

private:
  boost::unordered_flat_map<uint32_t, functor> _subscribers;
  sol::object _value;
  uint32_t _next_id{0};
};

class kv final {
public:
  ~kv() = default;
  std::shared_ptr<observable> get(std::string_view key, const sol::object& fallback = sol::lua_nil);
  void set(std::string_view key, const sol::object& value);

private:
  boost::unordered_flat_map<std::string, std::shared_ptr<observable>, transparent_string_hash, std::equal_to<>> _values;
};
