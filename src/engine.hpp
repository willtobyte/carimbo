#pragma once

#include "common.hpp"

class engine final : public eventreceiver {
public:
  engine() = default;
  virtual ~engine() = default;

  std::shared_ptr<::audiodevice> audiodevice() const noexcept;
  std::shared_ptr<::objectmanager> objectmanager() const noexcept;
  std::shared_ptr<::eventmanager> eventmanager() const noexcept;
  std::shared_ptr<::fontfactory> fontfactory() const noexcept;
  std::shared_ptr<::overlay> overlay() const noexcept;
  std::shared_ptr<::resourcemanager> resourcemanager() const noexcept;
  std::shared_ptr<::postalservice> postalservice() const noexcept;
  std::shared_ptr<::scenemanager> scenemanager() const noexcept;
  std::shared_ptr<::statemanager> statemanager() const noexcept;
  std::shared_ptr<::soundmanager> soundmanager() const noexcept;
  std::shared_ptr<::window> window() const noexcept;
  std::shared_ptr<::renderer> renderer() const noexcept;
  std::shared_ptr<::canvas> canvas() const noexcept;
  std::shared_ptr<::particlesystem> particlesystem() const noexcept;
  std::shared_ptr<::cassette> cassette() const noexcept;
  std::shared_ptr<::timermanager> timermanager() const noexcept;
  std::shared_ptr<::world> world() const noexcept;

  void set_audiodevice(std::shared_ptr<::audiodevice> ptr);
  void set_objectmanager(std::shared_ptr<::objectmanager> ptr);
  void set_eventmanager(std::shared_ptr<::eventmanager> ptr);
  void set_overlay(std::shared_ptr<::overlay> ptr);
  void set_resourcemanager(std::shared_ptr<::resourcemanager> ptr);
  void set_scenemanager(std::shared_ptr<::scenemanager> ptr);
  void set_particlesystem(std::shared_ptr<::particlesystem> ptr);
  void set_postalservice(std::shared_ptr<::postalservice> ptr);
  void set_statemanager(std::shared_ptr<::statemanager> ptr);
  void set_window(std::shared_ptr<::window> ptr);
  void set_world(std::shared_ptr<::world> ptr);
  void set_renderer(std::shared_ptr<::renderer> ptr);
  void set_timermanager(std::shared_ptr<::timermanager> ptr);

  void add_loopable(std::shared_ptr<::loopable> ptr);

  void run();

  void _loop();

protected:
  virtual void on_quit() noexcept override;

private:
  std::exception_ptr _exception;

#ifndef NDEBUG
  virtual void on_debug() override;
#endif

private:
  std::atomic<bool> _running{true};
  std::shared_ptr<::audiodevice> _audiodevice;
  std::shared_ptr<::objectmanager> _objectmanager;
  std::shared_ptr<::eventmanager> _eventmanager;
  std::vector<std::shared_ptr<::loopable>> _loopables;
  std::shared_ptr<::overlay> _overlay;
  std::shared_ptr<::resourcemanager> _resourcemanager;
  std::shared_ptr<::postalservice> _postalservice;
  std::shared_ptr<::scenemanager> _scenemanager;
  std::shared_ptr<::statemanager> _statemanager;
  std::shared_ptr<::renderer> _renderer;
  std::shared_ptr<::canvas> _canvas;
  std::shared_ptr<::particlesystem> _particlesystem;
  std::shared_ptr<::window> _window;
  std::shared_ptr<::world> _world;
  std::vector<std::shared_ptr<::lifecycleobserver>> _observers;
  std::shared_ptr<::timermanager> _timermanager;
  std::shared_ptr<::cassette> _cassette = std::make_shared<::cassette>();
};
