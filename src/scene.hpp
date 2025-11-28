#pragma once

#include "common.hpp"

#include "soundfx.hpp"
#include "particlesystem.hpp"
#include <entt/entity/fwd.hpp>

class entityproxy;

enum class scenekind : uint8_t {
  object = 0,
  effect,
  particle
};

class scene {
template <class OutIt>
[[nodiscard]] static bool collect(const b2ShapeId shape, void* const context) {
  auto* const it = static_cast<OutIt*>(context);
  const auto body = b2Shape_GetBody(shape);
  const auto data = b2Body_GetUserData(body);
  const auto id = userdata_to_id(data);
  **it = id;
  ++(*it);
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

  void set_onenter(std::function<void()>&& fn);
  void set_onloop(sol::protected_function fn);
  void set_oncamera(sol::protected_function fn);
  void set_onleave(std::function<void()>&& fn);
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

protected:
template <class OutIt>
  void query(const float x, const float y, OutIt out) const {
    auto aabb = b2AABB{};
    aabb.lowerBound = b2Vec2(x - epsilon, y - epsilon);
    aabb.upperBound = b2Vec2(x + epsilon, y + epsilon);
    const auto filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &collect<OutIt>, &out);
  }

  std::optional<entt::entity> find(uint64_t id) const;

private:
  entt::registry _registry;

  b2WorldId _world;

  std::shared_ptr<pixmap> _background;

  std::shared_ptr<renderer> _renderer;

  mutable std::unordered_set<uint64_t> _hovering;

  std::shared_ptr<scenemanager> _scenemanager;

  std::unordered_map<std::string, std::shared_ptr<soundfx>, string_hash, string_equal> _effects;
  std::unordered_map<std::string, std::shared_ptr<entityproxy>, string_hash, string_equal> _proxies;
  std::unordered_map<std::string, std::shared_ptr<particlebatch>, string_hash, string_equal> _particles;

  std::function<void()> _onenter;
  std::function<void()> _onleave;
  std::function<void(float)> _onloop;
  std::function<quad(float)> _oncamera;
  std::function<void(float, float)> _ontouch;
  std::function<void(int32_t)> _onkeypress;
  std::function<void(int32_t)> _onkeyrelease;
  std::function<void(std::string_view)> _ontext;
  std::function<void(float, float)> _onmotion;
};



// enum class scenekind : uint8_t {
//   object = 0,
//   effect,
//   particle
// };

// class scene {
// public:
//   explicit scene(std::string_view name, const nlohmann::json& j, std::shared_ptr<scenemanager> scenemanager);

//   virtual ~scene() noexcept;

//   virtual void update(float delta);

//   virtual void draw() const;

//   std::string_view name() const noexcept;

  // std::variant<
  //   std::shared_ptr<object>,
  //   std::shared_ptr<soundfx>,
  //   std::shared_ptr<particleprops>
  // > get(std::string_view name, scenekind type) const;

  // void on_enter() const;
  // void on_leave() const;
  // void on_text(std::string_view text) const;
  // void on_touch(float x, float y) const;
  // void on_key_press(int32_t code) const;
  // void on_key_release(int32_t code) const;
  // void on_motion(float x, float y) const;

  // void set_onenter(std::function<void()>&& fn);
  // void set_onloop(sol::protected_function fn);
  // void set_oncamera(sol::protected_function fn);
  // void set_onleave(std::function<void()>&& fn);
  // void set_ontouch(sol::protected_function fn);
  // void set_onkeypress(sol::protected_function fn);
  // void set_onkeyrelease(sol::protected_function fn);
  // void set_ontext(sol::protected_function fn);
  // void set_onmotion(sol::protected_function fn);

// protected:
//   void load();

//   std::string _name;
//   nlohmann::json _j;

//   quad _camera;
//   std::vector<std::pair<std::string, std::shared_ptr<object>>> _objects;
//   std::vector<std::pair<std::string, std::shared_ptr<soundfx>>> _effects;
//   std::unordered_map<std::string, std::shared_ptr<particlebatch>> _particles;

//   std::shared_ptr<scenemanager> _scenemanager;

//   std::shared_ptr<objectmanager> _objectmanager;
//   std::shared_ptr<particlesystem> _particlesystem;
//   std::shared_ptr<resourcemanager> _resourcemanager;

//   std::function<void()> _onenter;
//   std::function<void(float)> _onloop;
//   std::function<quad(float)> _oncamera;
//   std::function<void()> _onleave;
//   std::function<void(float, float)> _ontouch;
//   std::function<void(int32_t)> _onkeypress;
//   std::function<void(int32_t)> _onkeyrelease;
//   std::function<void(std::string_view)> _ontext;
//   std::function<void(float, float)> _onmotion;
// };

// class scenebackdrop : public scene {
// public:
//   scenebackdrop(std::string_view name, const nlohmann::json& j, std::shared_ptr<scenemanager> scenemanager);

//   virtual ~scenebackdrop() noexcept = default;

//   virtual void draw() const noexcept override;

// private:
//   std::shared_ptr<pixmap> _background;
// };
