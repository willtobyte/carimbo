#pragma once

#include "common.hpp"

enum scenetype : std::uint8_t {
  backdrop,
  // tilemap
};

[[nodiscard]] inline scenetype parse_scenetype(std::string_view str) noexcept {
  if (str == "backdrop") return scenetype::backdrop;
  // if (str == "tilemap") return scenetype::tilemap;
  return scenetype::backdrop;
}

class scenemanager final : public eventreceiver, public std::enable_shared_from_this<scenemanager> {
public:
  scenemanager(std::shared_ptr<::resourcemanager> resourcemanager, std::shared_ptr<::renderer> renderer);

  ~scenemanager() noexcept = default;

  std::shared_ptr<::scene> load(std::string_view name);

  std::string_view current() const;

  void set(std::string_view name);

  std::shared_ptr<::scene> get() const;

  boost::container::small_vector<std::string, 8> query(std::string_view name) const;

  boost::container::small_vector<std::string, 8> destroy(std::string_view name);

  void update(float delta);

  void draw() const;

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
  std::shared_ptr<::renderer> _renderer;
  boost::unordered_flat_map<std::string, std::shared_ptr<::scene>, transparent_string_hash, std::equal_to<>> _scene_mapping;
  std::weak_ptr<::scene> _scene;
  std::string _current;
};
