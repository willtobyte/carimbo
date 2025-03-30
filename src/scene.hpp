#pragma once

#include "common.hpp"

namespace framework {
class scene {
public:
  scene() = delete;
  scene(std::shared_ptr<graphics::pixmap> background, std::unordered_map<std::string, std::shared_ptr<object>> objects, geometry::size size) noexcept;
  ~scene() noexcept = default;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

  std::shared_ptr<object> get(const std::string &name) noexcept;

  void on_enter() noexcept;
  void on_leave() noexcept;

  void set_onenter(std::function<void()> fn) noexcept;
  void set_onloop(std::function<void(float_t)> fn) noexcept;
  void set_onleave(std::function<void()> fn) noexcept;

private:
  std::shared_ptr<graphics::pixmap> _background;
  std::unordered_map<std::string, std::shared_ptr<object>> _objects;
  geometry::size _size;

  std::function<void()> _onenter;
  std::function<void(float_t)> _onloop;
  std::function<void()> _onleave;
};
}
