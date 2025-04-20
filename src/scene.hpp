#pragma once

#include "common.hpp"

#include "object.hpp"
#include "pixmap.hpp"
#include "size.hpp"

namespace framework {
class objectmanager;

class scene final {
public:
  scene() = delete;
  explicit scene(
      std::shared_ptr<framework::objectmanager> objectmanager,
      std::shared_ptr<graphics::pixmap> background,
      std::unordered_map<std::string, std::shared_ptr<object>> objects,
      std::vector<std::shared_ptr<audio::soundfx>> effects,
      geometry::size size
  ) noexcept;

  ~scene() noexcept;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

  std::shared_ptr<object> get(const std::string &name) const noexcept;

  void on_enter() noexcept;
  void on_leave() noexcept;
  void on_touch(float_t x, float_t y) const noexcept;
  void on_motion(float_t x, float_t y) const noexcept;

  void set_onenter(std::function<void()> fn) noexcept;
  void set_onloop(std::function<void(float_t)> fn) noexcept;
  void set_onleave(std::function<void()> fn) noexcept;
  void set_ontouch(std::function<void(float_t, float_t)> fn) noexcept;
  void set_onmotion(std::function<void(float_t, float_t)> fn) noexcept;

private:
  std::shared_ptr<framework::objectmanager> _objectmanager;
  std::shared_ptr<graphics::pixmap> _background;
  std::unordered_map<std::string, std::shared_ptr<object>> _objects;
  std::vector<std::shared_ptr<audio::soundfx>> _effects;
  geometry::size _size;

  std::function<void()> _onenter;
  std::function<void(float_t)> _onloop;
  std::function<void()> _onleave;
  std::function<void(float_t, float_t)> _ontouch;
  std::function<void(float_t, float_t)> _onmotion;
};
}
