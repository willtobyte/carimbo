#pragma once

#include "common.hpp"
#include <boost/static_string/static_string.hpp>

class scenemanager final : public eventreceiver, public std::enable_shared_from_this<scenemanager> {
public:
  explicit scenemanager(std::shared_ptr<::renderer> renderer);

  ~scenemanager() noexcept = default;

  std::shared_ptr<::scene> load(std::string_view name);

  std::string_view current() const;

  void set(std::string_view name);

  std::vector<std::string_view> query(std::string_view name) const;

  std::vector<std::string_view> destroy(std::string_view name);

  void update(float delta);

  void draw() const;

  void on_tick(uint8_t tick);

  std::shared_ptr<::renderer> renderer() const noexcept;

  void set_runtime(sol::state_view runtime);

protected:
  virtual void on_key_press(const event::keyboard::key& event) override;
  virtual void on_key_release(const event::keyboard::key& event) override;
  virtual void on_text(std::string_view text) override;
  virtual void on_mouse_press(const event::mouse::button& event) override;
  virtual void on_mouse_release(const event::mouse::button& event) override;
  virtual void on_mouse_motion(const event::mouse::motion& event) override;

private:
  std::shared_ptr<::renderer> _renderer;
  sol::environment _environment;
  boost::unordered_flat_map<std::string, std::shared_ptr<::scene>, transparent_string_hash, std::equal_to<>> _scene_mapping;
  std::shared_ptr<::scene> _scene;
  std::shared_ptr<::scene> _pending;
  boost::static_string<32> _current;
};
