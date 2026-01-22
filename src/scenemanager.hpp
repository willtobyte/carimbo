#pragma once

#include "common.hpp"

class scenemanager final : public eventreceiver {
public:
  scenemanager() = default;

  ~scenemanager() noexcept = default;

  std::shared_ptr<::scene> load(std::string_view name);

  std::string_view current() const;

  void set(std::string_view name);

  std::vector<std::string> query(std::string_view name) const;

  std::vector<std::string> destroy(std::string_view name);

  void update(float delta);

  void draw() const;

  void on_tick(uint8_t tick);

  void set_runtime(sol::state_view runtime);

  void set_fontpool(std::shared_ptr<::fontpool> fontpool) noexcept;

protected:
  virtual void on_key_press(const event::keyboard::key& event) override;
  virtual void on_key_release(const event::keyboard::key& event) override;
  virtual void on_text(std::string_view text) override;
  virtual void on_mouse_press(const event::mouse::button& event) override;
  virtual void on_mouse_release(const event::mouse::button& event) override;
  virtual void on_mouse_motion(const event::mouse::motion& event) override;

private:
  sol::environment _environment;
  std::shared_ptr<::fontpool> _fontpool;
  boost::unordered_flat_map<std::string, std::shared_ptr<::scene>, transparent_string_hash, std::equal_to<>> _scene_mapping;
  std::shared_ptr<::scene> _scene;
  std::shared_ptr<::scene> _pending;
};
