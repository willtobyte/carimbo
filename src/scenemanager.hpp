#pragma once

#include "common.hpp"

#include "scene.hpp"

namespace framework {
class scenemanager {
public:
  scenemanager() = delete;
  scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<objectmanager> objectmanager) noexcept;
  ~scenemanager() noexcept = default;

  std::shared_ptr<scene> load(const std::string &name) noexcept;

  void set(const std::string &name) noexcept;

  std::shared_ptr<scene> get(const std::string &name) const noexcept;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

private:
  std::shared_ptr<graphics::pixmappool> _pixmappool;
  std::shared_ptr<objectmanager> _objectmanager;
  std::unordered_map<std::string, std::shared_ptr<scene>> _scene_mapping;
  std::string _current_scene{};
};
}
