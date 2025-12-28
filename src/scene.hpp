#pragma once

#include "common.hpp"

#include "registries.hpp"
#include "systems.hpp"
#include "tilemap.hpp"

enum class scenekind : uint8_t {
  object = 0,
  effect,
  particle
};

class scene final {
[[nodiscard]] static bool collect(const b2ShapeId shape, void* const context) {
  auto* const container = static_cast<entt::dense_set<entt::entity>*>(context);
  const auto body = b2Shape_GetBody(shape);
  const auto data = b2Body_GetUserData(body);
  const auto entity = static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(data));
  container->insert(entity);
  return true;
}

public:
  scene(std::string_view name, unmarshal::document& document, std::shared_ptr<::scenemanager> scenemanager, sol::environment environment);

  ~scene() noexcept;

  void update(float delta);

  void draw() const noexcept;

  std::string_view name() const noexcept { return ""; }

  sol::object get(std::string_view name, scenekind kind, sol::this_state state) const;

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

  void on_enter();
  void on_leave();
  void on_text(std::string_view text);
  void on_touch(float x, float y);
  void on_motion(float x, float y);
  void on_key_press(int32_t code);
  void on_key_release(int32_t code);
  void on_mail(uint64_t to, uint64_t from, std::string_view body);

  std::shared_ptr<::timermanager> timermanager() const noexcept;

private:
  void query(const float x, const float y, entt::dense_set<entt::entity>& out) const {
    auto aabb = b2AABB{};
    aabb.lowerBound = b2Vec2(x - epsilon, y - epsilon);
    aabb.upperBound = b2Vec2(x + epsilon, y + epsilon);
    const auto filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &collect, &out);
  }

  std::shared_ptr<renderer> _renderer;

  entt::registry _registry;

  std::shared_ptr<::timermanager> _timermanager;

  animationsystem _animationsystem{_registry};
  rendersystem _rendersystem{_registry};
  scriptsystem _scriptsystem{_registry};

  b2WorldId _world{};
  physicssystem _physicssystem{_registry};

  effects _effects;
  particles _particles;
  objects _objects;

  std::variant<std::monostate, std::shared_ptr<pixmap>, tilemap> _layer;

  std::function<void()> _onenter;
  std::function<void()> _onleave;
  functor _onloop;
  functor _ontouch;
  functor _onkeypress;
  functor _onkeyrelease;
  functor _ontext;
  functor _onmotion;
  functor _oncamera;

  entt::dense_set<entt::entity> _hits;
  entt::dense_set<entt::entity> _hovering;

  quad _camera{};
};
