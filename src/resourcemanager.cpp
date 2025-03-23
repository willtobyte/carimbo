#include "resourcemanager.hpp"

using namespace framework;

resourcemanager::resourcemanager(std::shared_ptr<graphics::renderer> renderer, std::shared_ptr<audio::audiodevice> audiodevice) noexcept
    : _renderer(std::move(renderer)),
      _audiodevice(std::move(audiodevice)),
      _pixmappool(std::make_shared<graphics::pixmappool>(_renderer)),
      _soundmanager(std::make_shared<audio::soundmanager>(_audiodevice)),
      _fontfactory(std::make_shared<graphics::fontfactory>(_renderer)) {
  _handlers = {
      {".png", [this](const std::string &filename) { _pixmappool->get(filename); }},
      {".ogg", [this](const std::string &filename) { _soundmanager->get(filename); }},
      {".json", [this](const std::string &filename) { _fontfactory->get(filename); }}
  };
}

void resourcemanager::flush() noexcept {
  _pixmappool->flush();
  _soundmanager->flush();
  _fontfactory->flush();
}

void resourcemanager::prefetch(const std::vector<std::string> &filenames) noexcept {
  for (const auto &filename : filenames) {
    if (const auto position = filename.rfind('.'); position != std::string::npos) {
      const auto extension = filename.substr(position);
      if (const auto it = _handlers.find(extension); it != _handlers.end()) {
        it->second(filename);
      }
    }
  }
}

std::shared_ptr<graphics::renderer> resourcemanager::renderer() const noexcept {
  return _renderer;
}

std::shared_ptr<graphics::pixmappool> resourcemanager::pixmappool() const noexcept {
  return _pixmappool;
}

std::shared_ptr<audio::soundmanager> resourcemanager::soundmanager() const noexcept {
  return _soundmanager;
}

std::shared_ptr<graphics::fontfactory> resourcemanager::fontfactory() const noexcept {
  return _fontfactory;
}
