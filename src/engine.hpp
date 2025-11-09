#pragma once

#include "common.hpp"

#include "eventreceiver.hpp"

namespace audio {
  class audiodevice;
  class soundmanager;
}

namespace framework {
  class lifecycleobserver;
  class loopable;
  class objectmanager;
  class resourcemanager;
  class postalservice;
  class scenemanager;
  class statemanager;
  class timermanager;
  class world;
}

namespace graphics {
  class canvas;
  class fontfactory;
  class overlay;
  class renderer;
  class particlesystem;
  class window;
}

namespace input {
  class eventmanager;
}

namespace storage {
  class cassette;
}

namespace framework {
class engine final : public input::eventreceiver {
public:
  engine() = default;
  virtual ~engine() = default;

  std::shared_ptr<audio::audiodevice> audiodevice() const;
  std::shared_ptr<framework::objectmanager> objectmanager() const;
  std::shared_ptr<input::eventmanager> eventmanager() const;
  std::shared_ptr<graphics::fontfactory> fontfactory() const;
  std::shared_ptr<graphics::overlay> overlay() const;
  std::shared_ptr<framework::resourcemanager> resourcemanager() const;
  std::shared_ptr<framework::postalservice> postalservice() const;
  std::shared_ptr<framework::scenemanager> scenemanager() const;
  std::shared_ptr<framework::statemanager> statemanager() const;
  std::shared_ptr<audio::soundmanager> soundmanager() const;
  std::shared_ptr<graphics::window> window() const;
  std::shared_ptr<graphics::renderer> renderer() const;
  std::shared_ptr<graphics::canvas> canvas() const;
  std::shared_ptr<graphics::particlesystem> particlesystem() const;
  std::shared_ptr<storage::cassette> cassette() const;
  std::shared_ptr<framework::timermanager> timermanager() const;
  std::shared_ptr<graphics::camera> camera() const;

  int32_t height() const;
  int32_t width() const;

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
  void set_camera(std::shared_ptr<graphics::camera> camera);
  void set_renderer(std::shared_ptr<graphics::renderer> renderer);
  void set_timermanager(std::shared_ptr<framework::timermanager> timermanager);

  void add_loopable(std::shared_ptr<loopable> loopable);

  void run();

  void _loop();

protected:
  virtual void on_quit() override;

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
  std::shared_ptr<graphics::camera> _camera;
  std::vector<std::shared_ptr<framework::lifecycleobserver>> _observers;
  std::shared_ptr<framework::timermanager> _timermanager;
  std::shared_ptr<storage::cassette> _cassette = std::make_shared<storage::cassette>();
};
}
