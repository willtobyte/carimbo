#pragma once

#include "common.hpp"

namespace framework {
class engine final : public input::eventreceiver {
public:
  engine() = default;
  virtual ~engine() = default;

  std::shared_ptr<audio::audiodevice> audiodevice() const noexcept;
  std::shared_ptr<framework::objectmanager> objectmanager() const noexcept;
  std::shared_ptr<input::eventmanager> eventmanager() const noexcept;
  std::shared_ptr<graphics::fontfactory> fontfactory() const noexcept;
  std::shared_ptr<graphics::overlay> overlay() const noexcept;
  std::shared_ptr<framework::resourcemanager> resourcemanager() const noexcept;
  std::shared_ptr<framework::postalservice> postalservice() const noexcept;
  std::shared_ptr<framework::scenemanager> scenemanager() const noexcept;
  std::shared_ptr<framework::statemanager> statemanager() const noexcept;
  std::shared_ptr<audio::soundmanager> soundmanager() const noexcept;
  std::shared_ptr<graphics::window> window() const noexcept;
  std::shared_ptr<graphics::renderer> renderer() const noexcept;
  std::shared_ptr<graphics::canvas> canvas() const noexcept;
  std::shared_ptr<graphics::particlesystem> particlesystem() const noexcept;
  std::shared_ptr<storage::cassette> cassette() const noexcept;
  std::shared_ptr<framework::timermanager> timermanager() const noexcept;

  void set_audiodevice(std::shared_ptr<audio::audiodevice> audiodevice);
  void set_objectmanager(std::shared_ptr<framework::objectmanager> objectmanager);
  void set_eventmanager(std::shared_ptr<input::eventmanager> eventmanager);
  void set_overlay(std::shared_ptr<graphics::overlay> overlay);
  void set_resourcemanager(std::shared_ptr<framework::resourcemanager> resourcemanager);
  void set_scenemanager(std::shared_ptr<framework::scenemanager> scenemanager);
  void set_particlesystem(std::shared_ptr<graphics::particlesystem> particlesystem);
  void set_postalservice(std::shared_ptr<framework::postalservice> postalservice);
  void set_statemanager(std::shared_ptr<framework::statemanager> statemanager);
  void set_window(std::shared_ptr<graphics::window> window);
  void set_world(std::shared_ptr<framework::world> world);
  void set_renderer(std::shared_ptr<graphics::renderer> renderer);
  void set_timermanager(std::shared_ptr<framework::timermanager> timermanager);

  void add_loopable(std::shared_ptr<loopable> loopable);

  void run();

  void _loop();

protected:
  virtual void on_quit() noexcept override;

#ifndef NDEBUG
  virtual void on_debug() override;
#endif

private:
  std::atomic<bool> _running{true};
  std::shared_ptr<audio::audiodevice> _audiodevice;
  std::shared_ptr<framework::objectmanager> _objectmanager;
  std::shared_ptr<input::eventmanager> _eventmanager;
  std::vector<std::shared_ptr<loopable>> _loopables;
  std::shared_ptr<graphics::overlay> _overlay;
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<framework::postalservice> _postalservice;
  std::shared_ptr<framework::scenemanager> _scenemanager;
  std::shared_ptr<framework::statemanager> _statemanager;
  std::shared_ptr<graphics::renderer> _renderer;
  std::shared_ptr<graphics::canvas> _canvas;
  std::shared_ptr<graphics::particlesystem> _particlesystem;
  std::shared_ptr<graphics::window> _window;
  std::shared_ptr<framework::world> _world;
  std::vector<std::shared_ptr<framework::lifecycleobserver>> _observers;
  std::shared_ptr<framework::timermanager> _timermanager;
  std::shared_ptr<storage::cassette> _cassette = std::make_shared<storage::cassette>();
};
}
