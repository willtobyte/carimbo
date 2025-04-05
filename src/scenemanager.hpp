#pragma once

#include "common.hpp"

#include "resourcemanager.hpp"
#include "scene.hpp"

namespace framework {
class objectmanager;

class scenemanager {
public:
  scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<framework::objectmanager> objectmanager) noexcept;
  ~scenemanager() noexcept = default;

  std::shared_ptr<scene> load(const std::string &name) noexcept;

  void set(const std::string &name) noexcept;

  std::shared_ptr<scene> get(const std::string &name) const noexcept;

  void on_click(float_t x, float_t y) const noexcept;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

private:
  std::shared_ptr<graphics::pixmappool> _pixmappool;
  std::shared_ptr<framework::objectmanager> _objectmanager;
  std::unordered_map<std::string, std::shared_ptr<scene>> _scene_mapping;
  std::shared_ptr<scene> _scene;
};
}
