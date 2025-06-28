#pragma once

#include "common.hpp"

#include "object.hpp"
#include "pixmap.hpp"
#include "size.hpp"
#include "tilemap.hpp"

namespace framework {
class objectmanager;

enum class scenetype : uint8_t {
  object = 0,
  effect
};

class scene final {
public:
  scene() = delete;
  explicit scene(
      std::shared_ptr<framework::objectmanager> objectmanager,
      std::shared_ptr<graphics::pixmap> background,
      std::vector<std::pair<std::string, std::shared_ptr<object>>> objects,
      std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> effects,
      std::optional<std::shared_ptr<tilemap>> tilemap,
      geometry::size size
  );

  ~scene();

  void update(float_t delta) noexcept;

  void draw() const noexcept;

  std::variant<std::shared_ptr<object>, std::shared_ptr<audio::soundfx>> get(const std::string& name, scenetype type) const;

  void on_enter() const;
  void on_leave() const;
  void on_text(const std::string& text) const;
  void on_touch(float_t x, float_t y) const;
  void on_key_press(int32_t code) const;
  void on_key_release(int32_t code) const;
  // void on_mouse_press(float_t x, float_t y) const;
  // void on_mouse_relese(float_t x, float_t y) const;
  void on_motion(float_t x, float_t y) const;

  void set_onenter(std::function<void()>&& fn);
  void set_onloop(std::function<void(float_t)>&& fn);
  void set_onleave(std::function<void()>&& fn);
  void set_ontouch(std::function<void(float_t, float_t)>&& fn);
  void set_onkeypress(std::function<void(int32_t)>&& fn);
  void set_onkeyrelease(std::function<void(int32_t)>&& fn);
  void set_ontext(std::function<void(const std::string& )>&& fn);
  void set_onmotion(std::function<void(float_t, float_t)>&& fn);

private:
  std::shared_ptr<framework::objectmanager> _objectmanager;
  std::shared_ptr<graphics::pixmap> _background;
  std::vector<std::pair<std::string, std::shared_ptr<object>>> _objects;
  std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> _effects;
  std::optional<std::shared_ptr<tilemap>> _tilemap;

  geometry::size _size;

  std::function<void()> _onenter;
  std::function<void(float_t)> _onloop;
  std::function<void()> _onleave;
  std::function<void(float_t, float_t)> _ontouch;
  std::function<void(int32_t)> _onkeypress;
  std::function<void(int32_t)> _onkeyrelease;
  std::function<void(const std::string& )> _ontext;
  // std::function<void(float_t, float_t, int32_t button)> _onmousepress;
  // std::function<void(float_t, float_t, int32_t button)> _onmouserelease;
  std::function<void(float_t, float_t)> _onmotion;
};
}
