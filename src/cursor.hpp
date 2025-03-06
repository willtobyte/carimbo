#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "pixmap.hpp"
#include "resourcemanager.hpp"
#include "types.hpp"

namespace graphics {
class cursor : public input::eventreceiver {
public:
  explicit cursor(const std::string &name, std::shared_ptr<framework::resourcemanager> resourcemanager);
  virtual ~cursor() = default;

  /* Entity manager will also listen for some mouse events to trigger entity action, here we just take care of animation */
  virtual void on_mousemotion(const input::mousemotionevent &event) noexcept;
  virtual void on_mousebuttondown(const input::mousebuttonevent &event) noexcept;
  virtual void on_mousebuttonup(const input::mousebuttonevent &event) noexcept;

  void update(float_t delta) noexcept;
  void draw() const noexcept;

private:
  int32_t _x;
  int32_t _y;
  std::string _action;
  std::string _next_action;
  uint64_t _frame;
  uint64_t _last_frame;
  geometry::point _point;
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<graphics::pixmap> _spritesheet;
  std::unordered_map<std::string, graphics::animation> _animations;
};
}
