#include "resourcemanager.hpp"
#include "engine.hpp"

using namespace framework;

static const std::map<std::string, std::function<void(
  const std::string&,
  graphics::pixmappool&,
  audio::soundmanager&
)>> handlers = {
  {
    ".png",
    [](const std::string& filename, graphics::pixmappool& pixmappool, audio::soundmanager&) {
      pixmappool.get(filename);
    }
  },
  {
    ".ogg",
    [](const std::string& filename, graphics::pixmappool&, audio::soundmanager& soundmanager) {
      soundmanager.get(filename);
    }
  }
};

resourcemanager::resourcemanager(
  std::shared_ptr<graphics::renderer> renderer,
  std::shared_ptr<audio::audiodevice> audiodevice,
  std::shared_ptr<framework::engine> engine
)
    : _renderer(std::move(renderer)),
      _audiodevice(std::move(audiodevice)),
      _engine(std::move(engine)),
      _pixmappool(std::make_shared<graphics::pixmappool>(_renderer)),
      _soundmanager(std::make_shared<audio::soundmanager>(_audiodevice)),
      _fontfactory(std::make_shared<graphics::fontfactory>(_renderer, _pixmappool)) {
  // TODO
  // const auto loop = [ptr = std::weak_ptr<framework::engine>(_engine)] {
  //   auto e = ptr.lock();
  //   if (!e) [[unlikely]] {
  //     return;
  //   }

  //   e->_loop();
  // };

  // _pixmappool->set_loop(loop);
  // _soundmanager->set_loop(loop);
  // _fontfactory->set_loop(loop);
}

void resourcemanager::flush() noexcept {
  _pixmappool->flush();
  _soundmanager->flush();
  _fontfactory->flush();
}

void resourcemanager::update(float_t delta) noexcept {
  _soundmanager->update(delta);
}

void resourcemanager::prefetch() {
  const auto directory = "blobs";
  const auto filenames = storage::io::enumerate(directory);
  std::vector<std::string> f;
  f.reserve(filenames.size());
  for (const auto& filename : filenames) {
    f.emplace_back(std::format("{}/{}", directory, filename));
  }

  prefetch(f);
}

void resourcemanager::prefetch(const std::vector<std::string>& filenames) {
  for (const auto& filename : filenames) {
    if (const auto& position = filename.rfind('.'); position != std::string::npos) {
      const auto& extension = filename.substr(position);
      if (const auto it = handlers.find(extension); it != handlers.end()) {
        it->second(filename, *_pixmappool, *_soundmanager);
      }
    }
  }
}

#ifdef DEBUG
void resourcemanager::debug() const noexcept {
  _pixmappool->debug();
  _soundmanager->debug();
  _fontfactory->debug();
}
#endif

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
