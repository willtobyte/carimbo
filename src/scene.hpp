#pragma once

#include "common.hpp"

#include "objectpool.hpp"
#include "particlepool.hpp"
#include "physics.hpp"
#include "soundpool.hpp"
#include "systems.hpp"
#include "tilemap.hpp"

class scene final {
public:
  scene(std::string_view name, unmarshal::json node, std::shared_ptr<::renderer> renderer, std::shared_ptr<::fontpool> fontpool, sol::environment environment);

  ~scene() noexcept = default;

  void update(float delta);

  void draw() const noexcept;

  [[nodiscard]] std::string_view name() const noexcept;

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
  void query(float x, float y, std::unordered_set<entt::entity>& out) const {
    const auto aabb = physics::make_aabb(x - epsilon, y - epsilon, epsilon * 2.0f, epsilon * 2.0f);
    _world.overlap_aabb(aabb, physics::category::all, [&out](b2ShapeId, entt::entity entity) {
      out.emplace(entity);
      return true;
    });
  }

  using view_type = decltype(std::declval<entt::registry&>().view<tickable>());

  boost::static_string<48> _name;

  entt::registry _registry;
  physics::world _world;
  quad _camera{};
  std::shared_ptr<renderer> _renderer;

  view_type _view;
  animationsystem _animationsystem{_registry};
  physicssystem _physicssystem{_registry, _world};
  rendersystem _rendersystem{_registry};
  scriptsystem _scriptsystem{_registry};
  velocitysystem _velocitysystem{_registry};

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

  std::unordered_set<entt::entity> _hits;
  std::unordered_set<entt::entity> _hovering;

  sol::environment _environment;
  soundpool _soundpool;
  particlepool _particlepool;
  objectpool _objectpool;
};
