#pragma once

#include "common.hpp"
#include <boost/static_string/static_string.hpp>

class scenemanager final : public eventreceiver, public std::enable_shared_from_this<scenemanager> {
public:
  scenemanager(std::shared_ptr<::resourcemanager> resourcemanager, std::shared_ptr<::renderer> renderer);

  ~scenemanager() noexcept = default;

  std::shared_ptr<::scene> load(std::string_view name);

  std::string_view current() const;

  void set(std::string_view name);

  boost::container::small_vector<std::string, 8> query(std::string_view name) const;

  boost::container::small_vector<std::string, 8> destroy(std::string_view name);

  void update(float delta);

  void draw() const;

  std::shared_ptr<::resourcemanager> resourcemanager() const noexcept;

  std::shared_ptr<::renderer> renderer() const noexcept;

  void set_runtime(sol::state_view runtime) noexcept;

protected:
  virtual void on_key_press(const event::keyboard::key& event) override;
  virtual void on_key_release(const event::keyboard::key& event) override;
  virtual void on_text(std::string_view text) override;
  virtual void on_mouse_press(const event::mouse::button& event) override;
  virtual void on_mouse_release(const event::mouse::button& event) override;
  virtual void on_mouse_motion(const event::mouse::motion& event) override;
  virtual void on_mail(const event::mail& event) override;

private:
  std::shared_ptr<::resourcemanager> _resourcemanager;
  std::shared_ptr<::renderer> _renderer;
  sol::environment _environment;
  boost::unordered_flat_map<std::string, std::shared_ptr<::scene>, transparent_string_hash, std::equal_to<>> _scene_mapping;
  ::scene* _scene{nullptr};
  boost::static_string<32> _current;
};
