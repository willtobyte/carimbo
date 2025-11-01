#pragma once

#include "common.hpp"

#include "particlesystem.hpp"
#include "resourcemanager.hpp"
#include "eventreceiver.hpp"
#include "scene.hpp"
#include "timermanager.hpp"

namespace framework {
class objectmanager;

class scenemanager final  : public input::eventreceiver {
public:
  scenemanager(
    std::shared_ptr<framework::resourcemanager> resourcemanager,
    std::shared_ptr<framework::objectmanager> objectmanager,
    std::shared_ptr<graphics::particlesystem> particlesystem,
    std::shared_ptr<framework::timermanager> timermanager
  );

  ~scenemanager() noexcept = default;

  std::shared_ptr<scene> load(const std::string& name);

  std::string current() const noexcept;

  void set(const std::string& name);

  std::shared_ptr<scene> get() const noexcept;

  std::vector<std::string> destroy(const std::string& name) noexcept;

  void update(float delta) noexcept;

  void draw() const noexcept;

  void on_touch(float x, float y) const;

protected:
  virtual void on_key_press(const input::event::keyboard::key& event) override;
  virtual void on_key_release(const input::event::keyboard::key& event) override;
  virtual void on_text(const std::string& text) override;
  virtual void on_mouse_press(const input::event::mouse::button& event) override;
  virtual void on_mouse_release(const input::event::mouse::button& event) override;
  virtual void on_mouse_motion(const input::event::mouse::motion& event) override;

private:
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<framework::objectmanager> _objectmanager;
  std::shared_ptr<graphics::particlesystem> _particlesystem;
  std::shared_ptr<framework::timermanager> _timermanager;
  std::unordered_map<std::string, std::shared_ptr<scene>> _scene_mapping;
  std::weak_ptr<scene> _scene;
  std::string _current;
};
}
