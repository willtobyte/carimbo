#pragma once

#include "common.hpp"

#include "objectmanager.hpp"
#include "particlesystem.hpp"
#include "physics.hpp"
#include "soundmanager.hpp"
#include "systems.hpp"
#include "tilemap.hpp"

class scene final {
[[nodiscard]] static bool collect(const b2ShapeId shape, void* const context) {
  auto* const container = static_cast<entt::dense_set<entt::entity>*>(context);
  container->insert(physics::entity_from(shape));
  return true;
}

public:
  scene(std::string_view name, unmarshal::json node, std::shared_ptr<::scenemanager> scenemanager, sol::environment& environment);

  ~scene() noexcept;

  void update(float delta);

  void draw() const noexcept;

  [[nodiscard]] std::string_view name() const noexcept { return _name; }

  void populate(sol::table& pool) const;

  void set_onenter(std::function<void()>&& fn);
  void set_onloop(sol::protected_function&& fn);
  void set_onleave(std::function<void()>&& fn);
  void set_ontouch(sol::protected_function&& fn);
  void set_onkeypress(sol::protected_function&& fn);
  void set_onkeyrelease(sol::protected_function&& fn);
  void set_ontext(sol::protected_function&& fn);
  void set_onmotion(sol::protected_function&& fn);
  void set_oncamera(sol::protected_function&& fn);
  void set_ontick(sol::protected_function&& fn);

  void on_enter();
  void on_leave();
  void on_text(std::string_view text);
  void on_touch(float x, float y);
  void on_motion(float x, float y);
  void on_key_press(int32_t code);
  void on_key_release(int32_t code);
  void on_tick(uint8_t tick);

private:
  void query(float x, float y, entt::dense_set<entt::entity>& out) const {
    auto aabb = b2AABB{};
    aabb.lowerBound = b2Vec2(x - epsilon, y - epsilon);
    aabb.upperBound = b2Vec2(x + epsilon, y + epsilon);
    const auto filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &collect, &out);
  }

  entt::registry _registry;
  b2WorldId _world{};
  quad _camera{};
  std::shared_ptr<renderer> _renderer;

  animationsystem _animationsystem{_registry};
  physicssystem _physicssystem{_registry};
  rendersystem _rendersystem{_registry};
  scriptsystem _scriptsystem{_registry};

  std::variant<std::monostate, std::shared_ptr<pixmap>, tilemap> _layer;

  functor _onloop;
  functor _oncamera;

  functor _ontouch;
  functor _onmotion;
  functor _onkeypress;
  functor _onkeyrelease;
  functor _ontext;
  functor _ontick;
  std::function<void()> _onenter;
  std::function<void()> _onleave;

  entt::dense_set<entt::entity> _hits;
  entt::dense_set<entt::entity> _hovering;

  std::string _name;
  soundmanager _soundmanager;
  particlesystem _particlesystem;
  objectmanager _objectmanager;
};
