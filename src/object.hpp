#pragma once

#include "common.hpp"
#include "kv.hpp"
#include "objectmanager.hpp"
#include "objectprops.hpp"
#include "reflection.hpp"
#include "vector2d.hpp"
#include <cstdint>

namespace framework {
class object : public std::enable_shared_from_this<object> {
public:
  explicit object(const objectprops &props) noexcept;
  ~object() noexcept;

  uint64_t id() const noexcept;

  std::string kind() const noexcept;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

  objectprops &props() noexcept;
  const objectprops &props() const noexcept;
  void set_props(const objectprops &props) noexcept;

  void hide() noexcept;

  geometry::point position() const noexcept;
  int32_t x() const noexcept;
  int32_t y() const noexcept;

  void move(float_t x_velocity, float_t y_velocity) noexcept;
  void set_velocity(const algebra::vector2d &velocity) noexcept;
  algebra::vector2d velocity() const noexcept;

  void set_placement(int32_t x, int32_t y) noexcept;
  geometry::point get_placement() const noexcept;

  void set_onupdate(std::function<void(std::shared_ptr<object>)> fn) noexcept;
  void set_onanimationfinished(std::function<void(std::shared_ptr<object>, const std::string &)> fn) noexcept;
  void set_onmail(std::function<void(std::shared_ptr<object>, const std::string &)> fn) noexcept;
  void set_ontouch(std::function<void()> fn) noexcept;
  void set_oncollision(const std::string &kind, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)> fn) noexcept;
  void set_onnthtick(uint64_t n, std::function<void(std::shared_ptr<object>)> fn) noexcept;

  void set_reflection(graphics::reflection reflection) noexcept;

  void set_action(const std::string &action) noexcept;
  void unset_action() noexcept;
  std::string action() const noexcept;

  bool intersects(std::shared_ptr<object> other) const noexcept;

  void on_email(const std::string &message);

  void on_touch() noexcept;

  memory::kv &kv() noexcept;

private:
  friend class objectmanager;

  memory::kv _kv;
  objectprops _props;
  uint64_t _tick_count{0};
  uint64_t _last_tick{0};
  std::function<void()> _ontouch;
  std::function<void(std::shared_ptr<object>)> _onupdate;
  std::function<void(std::shared_ptr<object>, const std::string &)> _onanimationfinished;
  std::function<void(std::shared_ptr<object>, const std::string &)> _onmail;
  std::unordered_map<std::string, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>> _collisionmapping;
  std::unordered_map<uint64_t, std::function<void(std::shared_ptr<object>)>> _tickinmapping;
};
}
