#pragma once

#include "common.hpp"

#include "scene.hpp"

namespace framework {
class scenemanager {
public:
  scenemanager() = delete;
  scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<objectmanager> objectmanager) noexcept;
  ~scenemanager() noexcept = default;

  void load(const std::string &name) noexcept;

  void set(const std::string &name) noexcept;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

  // std::shared_ptr<object> grab(const std::string &name, const std::string &key) const noexcept;

  void set_onenter(std::string name, std::function<void()> fn);
  void set_onloop(std::string name, std::function<void(float_t)> fn);
  void set_onleave(std::string name, std::function<void()> fn);

private:
  std::shared_ptr<graphics::pixmappool> _pixmappool;
  std::shared_ptr<objectmanager> _objectmanager;
  // std::unordered_map<std::string, std::shared_ptr<object>> _objects;
  // std::shared_ptr<graphics::pixmap> _background;
  std::unordered_map<std::string, std::function<void()>> _onenter_mapping;
  std::unordered_map<std::string, std::function<void(float_t)>> _onloop_mapping;
  std::unordered_map<std::string, std::function<void()>> _onleave_mapping;
  // geometry::size _size;
  std::unordered_map<std::string, std::shared_ptr<scene>> _scene_mapping;
  std::string _current_scene{};
};
}
