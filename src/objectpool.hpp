#pragma once

#include "common.hpp"

#include "physics.hpp"

class objectpool final {
public:
  objectpool(
      entt::registry& registry,
      physics::world& world,
      std::string_view scenename,
      sol::environment& environment
  );

  ~objectpool() noexcept = default;

  void add(unmarshal::json node, int32_t z);

  void populate(sol::table& pool) const;

  void sort();

private:
  struct shared {
    std::shared_ptr<const atlas> atlas;
    std::shared_ptr<pixmap> pixmap;
    float scale;
  };

  entt::registry& _registry;
  physics::world& _world;
  boost::static_string<48> _scenename;
  sol::environment& _environment;

  boost::unordered_flat_map<std::string, shared, transparent_string_hash, std::equal_to<>> _shared;
};
