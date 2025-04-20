#pragma once

#include "common.hpp"

#include "resourcemanager.hpp"
#include "scene.hpp"

namespace framework {
class objectmanager;

class scenemanager final {
public:
  scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<framework::objectmanager> objectmanager);
  ~scenemanager() = default;

  std::shared_ptr<scene> load(const std::string &name);

  void set(const std::string &name);

  std::shared_ptr<scene> get(const std::string &name) const;

  void destroy(const std::string &name);

  void on_touch(float_t x, float_t y) const;

  void on_motion(float_t x, float_t y) const;

  void update(float_t delta);

  void draw() const;

private:
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<framework::objectmanager> _objectmanager;
  std::unordered_map<std::string, std::shared_ptr<scene>> _scene_mapping;
  std::shared_ptr<scene> _scene;
};
}
