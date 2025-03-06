#pragma once

#include "common.hpp"
#include "entitymanager.hpp"
#include "entityprops.hpp"
#include "kv.hpp"
#include "reflection.hpp"
#include "vector2d.hpp"

namespace framework {
class entity : public std::enable_shared_from_this<entity> {
public:
  explicit entity(entityprops &&props) noexcept;
  ~entity() noexcept;

  static std::shared_ptr<entity> create(entityprops &&props);

  uint64_t id() const noexcept;
  std::string kind() const noexcept;

  virtual void update(float_t delta) noexcept;
  virtual void draw() const noexcept;

  entityprops &props() noexcept;
  const entityprops &props() const noexcept;
  void set_props(entityprops props) noexcept;

  geometry::point position() const noexcept;
  int32_t x() const noexcept;
  int32_t y() const noexcept;

  void move(float_t x_velocity, float_t y_velocity) noexcept;
  void set_velocity(const algebra::vector2d &velocity) noexcept;
  algebra::vector2d velocity() const noexcept;

  void set_placement(int32_t x, int32_t y) noexcept;
  geometry::point get_placement() const noexcept;

  void set_onupdate(std::function<void(std::shared_ptr<entity>)> fn) noexcept;
  void set_onanimationfinished(std::function<void(std::shared_ptr<entity>, const std::string &)> fn) noexcept;
  void set_onmail(std::function<void(std::shared_ptr<entity>, const std::string &)> fn) noexcept;
  void set_ontouch(std::function<void()> fn) noexcept;
  void set_oncollision(const std::string &kind, std::function<void(const std::shared_ptr<entity> &, const std::shared_ptr<entity> &)> fn) noexcept;

  void set_reflection(graphics::reflection reflection) noexcept;
  void set_action(const std::string &action) noexcept;
  std::string get_action() const noexcept;
  void unset_action() noexcept;

  std::string action() const noexcept;
  bool visible() const noexcept;

  bool intersects(const std::shared_ptr<entity> &other) const noexcept;

  void on_email(const std::string &message);

  void on_touch() noexcept;

  memory::kv &kv() noexcept;

private:
  friend class entitymanager;

  entityprops _props;
  memory::kv _kv;
  std::function<void(std::shared_ptr<entity>)> _onupdate;
  std::function<void(std::shared_ptr<entity>, const std::string &)> _onanimationfinished;
  std::function<void(std::shared_ptr<entity>, const std::string &)> _onmail;
  std::function<void()> _ontouch;
  std::unordered_map<std::string, std::function<void(const std::shared_ptr<entity> &, const std::shared_ptr<entity> &)>> _collisionmapping;
};
}
