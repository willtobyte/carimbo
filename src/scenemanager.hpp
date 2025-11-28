#pragma once

#include "common.hpp"

enum scenetype : std::uint8_t {
  backdrop,
  // tilemap
};

NLOHMANN_JSON_SERIALIZE_ENUM(scenetype, {
  {scenetype::backdrop, "backdrop"},
  // {scenetype::tilemap, "tilemap"},
})

class scenemanager final : public eventreceiver, public std::enable_shared_from_this<scenemanager> {
public:
  scenemanager(
    std::shared_ptr<::resourcemanager> resourcemanager,
    std::shared_ptr<::particlesystem> particlesystem,
    std::shared_ptr<::timermanager> timermanager,
    std::shared_ptr<::renderer> renderer
  );

  ~scenemanager() = default;

  std::shared_ptr<::scene> load(std::string_view name);

  std::string_view current() const;

  void set(std::string_view name);

  std::shared_ptr<::scene> get() const;

  std::vector<std::string> query(std::string_view name) const;

  std::vector<std::string> destroy(std::string_view name);

  void update(float delta);

  void draw() const;

  void on_touch(float x, float y) const;

  std::shared_ptr<::objectmanager> objectmanager() const noexcept;
  std::shared_ptr<::particlesystem> particlesystem() const noexcept;
  std::shared_ptr<::resourcemanager> resourcemanager() const noexcept;
  std::shared_ptr<::renderer> renderer() const noexcept;

protected:
  virtual void on_key_press(const event::keyboard::key& event) override;
  virtual void on_key_release(const event::keyboard::key& event) override;
  virtual void on_text(std::string_view text) override;
  virtual void on_mouse_press(const event::mouse::button& event) override;
  virtual void on_mouse_release(const event::mouse::button& event) override;
  virtual void on_mouse_motion(const event::mouse::motion& event) override;

private:
  std::shared_ptr<::resourcemanager> _resourcemanager;
  std::shared_ptr<::objectmanager> _objectmanager;
  std::shared_ptr<::particlesystem> _particlesystem;
  std::shared_ptr<::timermanager> _timermanager;
  std::shared_ptr<::renderer> _renderer;
  std::unordered_map<std::string, std::shared_ptr<::scene>, string_hash, string_equal> _scene_mapping;
  std::weak_ptr<::scene> _scene;
  std::string _current;
};
