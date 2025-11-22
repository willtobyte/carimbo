#pragma once

#include "common.hpp"

namespace framework {
enum scenetype : std::uint8_t {
  backdrop,
  // tilemap
};

NLOHMANN_JSON_SERIALIZE_ENUM(scenetype, {
  {scenetype::backdrop, "backdrop"},
  // {scenetype::tilemap, "tilemap"},
})

class scenemanager final : public input::eventreceiver, public std::enable_shared_from_this<scenemanager> {
public:
  scenemanager(
    std::shared_ptr<framework::resourcemanager> resourcemanager,
    std::shared_ptr<framework::objectmanager> objectmanager,
    std::shared_ptr<graphics::particlesystem> particlesystem,
    std::shared_ptr<framework::timermanager> timermanager
  );

  ~scenemanager() = default;

  std::shared_ptr<scene> load(std::string_view name);

  std::string_view current() const;

  void set(std::string_view name);

  std::shared_ptr<scene> get() const;

  std::vector<std::string> query(std::string_view name) const;

  std::vector<std::string> destroy(std::string_view name);

  void update(float delta);

  void draw() const;

  void on_touch(float x, float y) const;

  std::shared_ptr<objectmanager> objectmanager() const noexcept;
  std::shared_ptr<graphics::particlesystem> particlesystem() const noexcept;
  std::shared_ptr<resourcemanager> resourcemanager() const noexcept;

protected:
  virtual void on_key_press(const input::event::keyboard::key& event) override;
  virtual void on_key_release(const input::event::keyboard::key& event) override;
  virtual void on_text(std::string_view text) override;
  virtual void on_mouse_press(const input::event::mouse::button& event) override;
  virtual void on_mouse_release(const input::event::mouse::button& event) override;
  virtual void on_mouse_motion(const input::event::mouse::motion& event) override;

private:
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<framework::objectmanager> _objectmanager;
  std::shared_ptr<graphics::particlesystem> _particlesystem;
  std::shared_ptr<framework::timermanager> _timermanager;
  std::unordered_map<std::string, std::shared_ptr<scene>, string_hash, string_equal> _scene_mapping;
  std::weak_ptr<scene> _scene;
  std::string _current;
};
}
