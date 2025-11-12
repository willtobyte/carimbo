#pragma once

#include "common.hpp"

namespace audio {
  class soundfx;
}

namespace framework {
class objectmanager;
class tilemap;

enum class scenetype : uint8_t {
  object = 0,
  effect,
  particle
};

class scene final {
public:
  scene(
    std::string_view name,
    std::shared_ptr<framework::objectmanager> objectmanager,
    std::shared_ptr<graphics::particlesystem> particlesystem,
    std::shared_ptr<graphics::pixmap> background,
    std::vector<std::pair<std::string, std::shared_ptr<object>>> objects,
    std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> effects,
    std::unordered_map<std::string, std::shared_ptr<graphics::particlebatch>> particles,
    std::optional<std::shared_ptr<tilemap>> tilemap,
    geometry::size size
  );

  ~scene();

  void update(float delta);

  void draw() const;

  std::variant<
    std::shared_ptr<object>,
    std::shared_ptr<audio::soundfx>,
    std::shared_ptr<graphics::particleprops>
  > get(std::string_view name, scenetype type) const;

  std::string name() const;

  void on_enter() const;
  void on_leave() const;
  void on_text(std::string_view text) const;
  void on_touch(float x, float y) const;
  void on_key_press(int32_t code) const;
  void on_key_release(int32_t code) const;
  void on_motion(float x, float y) const;

  void set_onenter(sol::protected_function fn);
  void set_onloop(sol::protected_function fn);
  void set_onleave(sol::protected_function fn);
  void set_ontouch(sol::protected_function fn);
  void set_onkeypress(sol::protected_function fn);
  void set_onkeyrelease(sol::protected_function fn);
  void set_ontext(sol::protected_function fn);
  void set_onmotion(sol::protected_function fn);

private:
  std::string _name;
  std::shared_ptr<framework::objectmanager> _objectmanager;
  std::shared_ptr<graphics::particlesystem> _particlesystem;
  std::shared_ptr<graphics::pixmap> _background;
  std::vector<std::pair<std::string, std::shared_ptr<object>>> _objects;
  std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> _effects;
  std::unordered_map<std::string, std::shared_ptr<graphics::particlebatch>> _particles;
  std::optional<std::shared_ptr<tilemap>> _tilemap;

  geometry::size _size;

  std::function<void()> _onenter;
  std::function<void(float)> _onloop;
  std::function<void()> _onleave;
  std::function<void(float, float)> _ontouch;
  std::function<void(int32_t)> _onkeypress;
  std::function<void(int32_t)> _onkeyrelease;
  std::function<void(std::string_view)> _ontext;
  std::function<void(float, float)> _onmotion;
};
}
