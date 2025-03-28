#pragma once

#include "common.hpp"

namespace framework {
class scenemanager {
public:
  explicit scenemanager(std::shared_ptr<graphics::pixmappool> pixmappool, std::shared_ptr<entitymanager> entitymanager) noexcept;

  void set(const std::string &name) noexcept;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

  std::shared_ptr<entity> grab(const std::string &key) const noexcept;

  void set_onenter(const std::string &name, std::function<void()> fn);
  void set_onleave(const std::string &name, std::function<void()> fn);

private:
  std::shared_ptr<graphics::pixmappool> _pixmappool;
  std::shared_ptr<entitymanager> _entitymanager;
  std::unordered_map<std::string, std::shared_ptr<entity>> _entities;
  std::shared_ptr<graphics::pixmap> _background;
  std::unordered_map<std::string, std::function<void()>> _onenter_mapping;
  std::unordered_map<std::string, std::function<void()>> _onleave_mapping;
  geometry::size _size;
  std::string _current_scene{};
};
}
