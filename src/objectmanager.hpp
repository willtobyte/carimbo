#pragma once

#include "common.hpp"

class objectmanager final {
public:
  objectmanager(
      entt::registry& registry,
      std::shared_ptr<renderer> renderer,
      std::string_view scenename,
      sol::environment& environment
  );

  ~objectmanager() noexcept = default;

  void add(unmarshal::json node, int32_t z);

  void populate(sol::table& pool) const;

  void sort();

private:
  entt::registry& _registry;
  std::shared_ptr<renderer> _renderer;
  std::string _scenename;
  sol::environment& _environment;
  boost::unordered_flat_map<std::string, std::shared_ptr<objectproxy>, transparent_string_hash, std::equal_to<>> _proxies;
};
