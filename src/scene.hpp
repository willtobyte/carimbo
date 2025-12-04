#pragma once

#include "common.hpp"

#include "systems.hpp"

class entityproxy;

enum class scenekind : uint8_t {
  object = 0,
  effect,
  particle
};

class scene {
[[nodiscard]] static bool collect(const b2ShapeId shape, void* const context) {
  auto* const container = static_cast<entt::dense_set<entt::entity>*>(context);
  const auto body = b2Shape_GetBody(shape);
  const auto data = b2Body_GetUserData(body);
  const auto entity = static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(data));
  container->insert(entity);
  return true;
}

public:
  scene(std::string_view scene, const nlohmann::json& json, std::shared_ptr<scenemanager> scenemanager);

  ~scene() noexcept;

  void update(float delta) noexcept;

  void draw() const noexcept;

  std::string_view name() const noexcept { return ""; }

  std::variant<
    std::shared_ptr<entityproxy>,
    std::shared_ptr<soundfx>,
    std::shared_ptr<particleprops>
  > get(std::string_view name, scenekind kind) const;

  void set_onenter(sol::protected_function && fn);
  void set_onloop(sol::protected_function fn);
  void set_oncamera(sol::protected_function fn);
  void set_onleave(sol::protected_function&& fn);
  void set_ontouch(sol::protected_function fn);
  void set_onkeypress(sol::protected_function fn);
  void set_onkeyrelease(sol::protected_function fn);
  void set_ontext(sol::protected_function fn);
  void set_onmotion(sol::protected_function fn);

  void on_enter() const;
  void on_leave() const;
  void on_text(std::string_view text) const;
  void on_touch(float x, float y) const;
  void on_motion(float x, float y) const;
  void on_key_press(int32_t code) const;
  void on_key_release(int32_t code) const;

  std::shared_ptr<::timermanager> timermanager() const noexcept;

protected:
  void query(const float x, const float y, entt::dense_set<entt::entity>& out) const {
    auto aabb = b2AABB{};
    aabb.lowerBound = b2Vec2(x - epsilon, y - epsilon);
    aabb.upperBound = b2Vec2(x + epsilon, y + epsilon);
    const auto filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &collect, &out);
  }

private:
  entt::registry _registry;
  b2WorldId _world;
  animationsystem _animationsystem;
  physicssystem _physicssystem;
  rendersystem _rendersystem;
  particlesystem _particlesystem;

  std::shared_ptr<pixmap> _background;
  std::shared_ptr<renderer> _renderer;
  std::shared_ptr<::timermanager> _timermanager;

  std::shared_ptr<scenemanager> _scenemanager;

  std::unordered_map<std::string, std::shared_ptr<soundfx>, string_hash, string_equal> _effects;
  std::unordered_map<std::string, std::shared_ptr<entityproxy>, string_hash, string_equal> _proxies;
  std::unordered_map<std::string, std::shared_ptr<particlebatch>, string_hash, string_equal> _particles;

  functor _onenter;
  functor _onleave;
  functor _onloop;
  functor _oncamera;
  functor _ontouch;
  functor _onkeypress;
  functor _onkeyrelease;
  functor _ontext;
  functor _onmotion;

  mutable entt::dense_set<entt::entity> _hovering;
};
