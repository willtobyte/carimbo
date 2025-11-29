#pragma once

#include "common.hpp"

class engine final : public eventreceiver {
public:
  engine() noexcept = default;
  ~engine() noexcept = default;

  std::shared_ptr<::audiodevice> audiodevice() const noexcept;
  std::shared_ptr<::eventmanager> eventmanager() const noexcept;
  std::shared_ptr<::fontfactory> fontfactory() const noexcept;
  std::shared_ptr<::resourcemanager> resourcemanager() const noexcept;
  std::shared_ptr<::scenemanager> scenemanager() const noexcept;
  std::shared_ptr<::statemanager> statemanager() const noexcept;
  std::shared_ptr<::soundmanager> soundmanager() const noexcept;
  std::shared_ptr<::window> window() const noexcept;
  std::shared_ptr<::renderer> renderer() const noexcept;
  std::shared_ptr<::overlay> overlay() const noexcept;

  void set_audiodevice(std::shared_ptr<::audiodevice> ptr) noexcept;
  void set_eventmanager(std::shared_ptr<::eventmanager> ptr) noexcept;
  void set_resourcemanager(std::shared_ptr<::resourcemanager> ptr) noexcept;
  void set_scenemanager(std::shared_ptr<::scenemanager> ptr) noexcept;
  void set_statemanager(std::shared_ptr<::statemanager> ptr) noexcept;
  void set_window(std::shared_ptr<::window> ptr) noexcept;
  void set_renderer(std::shared_ptr<::renderer> ptr) noexcept;
  void set_overlay(std::shared_ptr<::overlay> ptr) noexcept;

  void add_loopable(std::shared_ptr<::loopable> ptr) noexcept;

  void run();

  void _loop();

protected:
  virtual void on_quit() noexcept override;

#ifndef NDEBUG
  virtual void on_debug() override;
#endif

private:
  std::atomic<bool> _running{true};
  std::shared_ptr<::audiodevice> _audiodevice;
  std::shared_ptr<::eventmanager> _eventmanager;
  std::vector<std::shared_ptr<::loopable>> _loopables;
  std::shared_ptr<::resourcemanager> _resourcemanager;
  std::shared_ptr<::scenemanager> _scenemanager;
  std::shared_ptr<::statemanager> _statemanager;
  std::shared_ptr<::renderer> _renderer;
  std::shared_ptr<::window> _window;
  std::shared_ptr<::overlay> _overlay;
  std::vector<std::shared_ptr<::lifecycleobserver>> _observers;
};
