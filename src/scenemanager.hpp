#pragma once

#include "common.hpp"

#include "resourcemanager.hpp"
#include "eventreceiver.hpp"
#include "scene.hpp"

namespace framework {
class objectmanager;

class scenemanager final  : public input::eventreceiver {
public:
  scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<framework::objectmanager> objectmanager);
  ~scenemanager() noexcept = default;

  std::shared_ptr<scene> load(const std::string& name);

  void set(const std::string& name);

  std::shared_ptr<scene> get(const std::string& name) const;

  void destroy(const std::string& name);

  void update(float_t delta) noexcept;

  void draw() const noexcept;

  void on_touch(float_t x, float_t y) const;

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
  std::unordered_map<std::string, std::shared_ptr<scene>> _scene_mapping;
  std::shared_ptr<scene> _scene;
};
}
