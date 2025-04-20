#include "resourcemanager.hpp"

using namespace framework;

static const std::map<std::string, std::function<void(const std::string &,
                                                      graphics::pixmappool &,
                                                      audio::soundmanager &,
                                                      graphics::fontfactory &)>> handlers = {
    {".png", [](const std::string &filename,
                graphics::pixmappool &pixmap,
                audio::soundmanager &,
                graphics::fontfactory &) {
       pixmap.get(filename);
    }},
    {".ogg", [](const std::string &filename,
                graphics::pixmappool &,
                audio::soundmanager &sound,
                graphics::fontfactory &) {
       sound.get(filename);
    }},
    {".json", [](const std::string &filename,
                 graphics::pixmappool &,
                 audio::soundmanager &,
                 graphics::fontfactory &font) {
       font.get(filename);
    }}
};

resourcemanager::resourcemanager(std::shared_ptr<graphics::renderer> renderer, std::shared_ptr<audio::audiodevice> audiodevice)
    : _renderer(std::move(renderer)),
      _audiodevice(std::move(audiodevice)),
      _pixmappool(std::make_shared<graphics::pixmappool>(_renderer)),
      _soundmanager(std::make_shared<audio::soundmanager>(_audiodevice)),
      _fontfactory(std::make_shared<graphics::fontfactory>(_renderer)) {
}

void resourcemanager::flush() {
  _pixmappool->flush();
  _soundmanager->flush();
  _fontfactory->flush();
}

void resourcemanager::prefetch() {
  const auto directory = "blobs";
  const auto filenames = storage::io::list(directory);
  std::vector<std::string> f;
  f.reserve(filenames.size());
  for (const auto &filename : filenames) {
    f.emplace_back(fmt::format("{}/{}", directory, filename));
  }

  prefetch(f);
}

void resourcemanager::prefetch(const std::vector<std::string> &filenames) {
  for (const auto &filename : filenames) {
    if (const auto position = filename.rfind('.'); position != std::string::npos) {
      const auto extension = filename.substr(position);
      if (const auto it = handlers.find(extension); it != handlers.end()) {
        it->second(filename, *_pixmappool, *_soundmanager, *_fontfactory);
      }
    }
  }
}

std::shared_ptr<graphics::renderer> resourcemanager::renderer() const {
  return _renderer;
}

std::shared_ptr<graphics::pixmappool> resourcemanager::pixmappool() const {
  return _pixmappool;
}

std::shared_ptr<audio::soundmanager> resourcemanager::soundmanager() const {
  return _soundmanager;
}

std::shared_ptr<graphics::fontfactory> resourcemanager::fontfactory() const {
  return _fontfactory;
}
