#pragma once

#include "common.hpp"

#include "audiodevice.hpp"
#include "canvas.hpp"
#include "cassette.hpp"
#include "eventmanager.hpp"
#include "eventreceiver.hpp"
#include "fontfactory.hpp"
#include "lifecycleobserver.hpp"
#include "loopable.hpp"
#include "objectmanager.hpp"
#include "overlay.hpp"
#include "renderer.hpp"
#include "resourcemanager.hpp"
#include "scenemanager.hpp"
#include "soundmanager.hpp"
#include "statemanager.hpp"
#include "timermanager.hpp"
#include "window.hpp"

namespace framework {
class engine : public input::eventreceiver {
public:
  engine();
  virtual ~engine() = default;

  std::shared_ptr<audio::audiodevice> audiodevice() const;
  std::shared_ptr<framework::objectmanager> objectmanager() const;
  std::shared_ptr<input::eventmanager> eventmanager() const;
  std::shared_ptr<graphics::fontfactory> fontfactory() const;
  std::shared_ptr<graphics::overlay> overlay() const;
  std::shared_ptr<framework::resourcemanager> resourcemanager() const;
  std::shared_ptr<framework::scenemanager> scenemanager() const;
  std::shared_ptr<framework::statemanager> statemanager() const;
  std::shared_ptr<audio::soundmanager> soundmanager() const;
  std::shared_ptr<graphics::window> window() const;
  std::shared_ptr<graphics::renderer> renderer() const;
  std::shared_ptr<graphics::canvas> canvas() const;
  std::shared_ptr<storage::cassette> cassette() const;
  std::shared_ptr<framework::timermanager> timermanager() const;

  int32_t height() const;
  int32_t width() const;

  void set_audiodevice(std::shared_ptr<audio::audiodevice> audiodevice);
  void set_objectmanager(std::shared_ptr<framework::objectmanager> objectmanager);
  void set_eventmanager(std::shared_ptr<input::eventmanager> eventmanager);
  void set_overlay(std::shared_ptr<graphics::overlay> overlay);
  void set_resourcemanager(std::shared_ptr<framework::resourcemanager> resourcemanager);
  void set_scenemanager(std::shared_ptr<framework::scenemanager> scenemanager);
  void set_statemanager(std::shared_ptr<framework::statemanager> statemanager);
  void set_window(std::shared_ptr<graphics::window> window);
  void set_renderer(std::shared_ptr<graphics::renderer> renderer);

  void add_loopable(std::shared_ptr<loopable> loopable);
  void flush() const;
  void prefetch();
  void prefetch(const std::vector<std::string> &filenames);
  void run();

  void _loop();

protected:
  virtual void on_quit() override;

private:
  std::atomic<bool> _running{true};
  std::shared_ptr<audio::audiodevice> _audiodevice;
  std::shared_ptr<framework::objectmanager> _objectmanager;
  std::shared_ptr<input::eventmanager> _eventmanager;
  std::vector<std::shared_ptr<loopable>> _loopables;
  std::shared_ptr<graphics::overlay> _overlay;
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<framework::scenemanager> _scenemanager;
  std::shared_ptr<framework::statemanager> _statemanager;
  std::shared_ptr<graphics::renderer> _renderer;
  std::shared_ptr<graphics::canvas> _canvas;
  std::shared_ptr<graphics::window> _window;
  std::vector<std::shared_ptr<lifecycleobserver>> _observers;
  std::shared_ptr<storage::cassette> _cassette = std::make_shared<storage::cassette>();
  std::shared_ptr<framework::timermanager> _timermanager = std::make_shared<framework::timermanager>();
};
}
