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

  void set_onupdate(const std::function<void(std::shared_ptr<entity>)> &fn) noexcept;
  void set_onanimationfinished(const std::function<void(std::shared_ptr<entity>)> &fn) noexcept;
  void set_onmail(const std::function<void(std::shared_ptr<entity>, const std::string &)> &fn) noexcept;
  void set_oncollision(const std::string &kind, const std::function<void(std::shared_ptr<entity>, uint64_t)> &fn) noexcept;

  void set_reflection(graphics::reflection reflection) noexcept;
  void set_action(const std::string &action);
  void unset_action();

  std::string action() const noexcept;
  geometry::size size() const noexcept;
  bool visible() const noexcept;

  void on_email(const std::string &message);

  memory::kv &kv() noexcept;

private:
  friend class entitymanager;

  entityprops _props;
  memory::kv _kv;
  std::function<void(std::shared_ptr<entity>)> _onupdate;
  std::function<void(std::shared_ptr<entity>)> _onanimationfinished;
  std::function<void(std::shared_ptr<entity>, const std::string &)> _onmail;
  std::unordered_map<std::string, std::function<void(std::shared_ptr<entity>, uint64_t)>> _collisionmapping;
};
}
