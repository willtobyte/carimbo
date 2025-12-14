#pragma once

#include "common.hpp"

#include "systems.hpp"

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
  scene(std::string_view scene, unmarshal::document& document, std::weak_ptr<::scenemanager> scenemanager);

  virtual ~scene() noexcept;

  virtual void update(float delta) noexcept;

  virtual void draw() const noexcept;

  std::string_view name() const noexcept { return ""; }

  virtual std::variant<
    std::shared_ptr<entityproxy>,
    std::shared_ptr<soundfx>,
    std::shared_ptr<particleprops>
  > get(std::string_view name, scenekind kind) const;

  virtual void set_onenter(std::function<void()>&& fn);
  virtual void set_onloop(sol::protected_function&& fn);
  virtual void set_onleave(std::function<void()>&& fn);
  virtual void set_ontouch(sol::protected_function&& fn);
  virtual void set_onkeypress(sol::protected_function&& fn);
  virtual void set_onkeyrelease(sol::protected_function&& fn);
  virtual void set_ontext(sol::protected_function&& fn);
  virtual void set_onmotion(sol::protected_function&& fn);
  virtual void set_oncamera(sol::protected_function&& fn);

  virtual void on_enter();
  virtual void on_leave();
  virtual void on_text(std::string_view text);
  virtual void on_touch(float x, float y);
  virtual void on_motion(float x, float y);
  virtual void on_key_press(int32_t code);
  virtual void on_key_release(int32_t code);

  std::shared_ptr<::timermanager> timermanager() const noexcept;

protected:
  void query(const float x, const float y, entt::dense_set<entt::entity>& out) const {
    auto aabb = b2AABB{};
    aabb.lowerBound = b2Vec2(x - epsilon, y - epsilon);
    aabb.upperBound = b2Vec2(x + epsilon, y + epsilon);
    const auto filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &collect, &out);
  }

  std::weak_ptr<scenemanager> _scenemanager;
  std::shared_ptr<renderer> _renderer;

private:
  entt::registry _registry;

  b2WorldId _world;

  animationsystem _animationsystem{_registry};
  physicssystem _physicssystem{_registry};
  rendersystem _rendersystem{_registry};
  std::shared_ptr<::timermanager> _timermanager;
  std::shared_ptr<pixmap> _background;

  particlesystem _particlesystem;

  boost::unordered_flat_map<std::string, std::shared_ptr<soundfx>, transparent_string_hash, std::equal_to<>> _effects;
  boost::unordered_flat_map<std::string, std::shared_ptr<entityproxy>, transparent_string_hash, std::equal_to<>> _proxies;
  boost::unordered_flat_map<std::string, std::shared_ptr<particlebatch>, transparent_string_hash, std::equal_to<>> _particles;

  std::function<void()> _onenter;
  std::function<void()> _onleave;
  functor _onloop;
  functor _ontouch;
  functor _onkeypress;
  functor _onkeyrelease;
  functor _ontext;
  functor _onmotion;

  entt::dense_set<entt::entity> _hits;
  entt::dense_set<entt::entity> _hovering;
};
