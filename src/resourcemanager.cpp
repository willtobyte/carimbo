#include "resourcemanager.hpp"

#include "audiodevice.hpp"
#include "engine.hpp"
#include "fontfactory.hpp"
#include "io.hpp"
#include "pixmappool.hpp"
#include "renderer.hpp"
#include "soundmanager.hpp"



resourcemanager::resourcemanager(
  std::shared_ptr<::renderer> renderer,
  std::shared_ptr<::audiodevice> audiodevice,
  std::shared_ptr<::engine> engine
)
    : _renderer(std::move(renderer)),
      _audiodevice(std::move(audiodevice)),
      _engine(std::move(engine)),
      _pixmappool(std::make_shared<::pixmappool>(_renderer)),
      _soundmanager(std::make_shared<::soundmanager>(_audiodevice)),
      _fontfactory(std::make_shared<::fontfactory>(_renderer, _pixmappool)) {


}

void resourcemanager::flush() {
  _pixmappool->flush();
  _soundmanager->flush();
  _fontfactory->flush();
}

void resourcemanager::update(float delta) {
  _soundmanager->update(delta);
}

void resourcemanager::prefetch() {
  static constexpr std::string_view directory = "blobs";
  const auto filenames = io::enumerate(directory);
  std::vector<std::string> f;
  f.reserve(filenames.size());
  std::string buffer;
  buffer.reserve(64);
  for (const auto& filename : filenames) {
    buffer.clear();
    std::format_to(std::back_inserter(buffer), "{}/{}", directory, filename);
    f.emplace_back(buffer);
  }

  prefetch(f);
}

void resourcemanager::prefetch(const std::vector<std::string>& filenames) {
  for (const auto& filename : filenames) {
    if (const auto position = filename.rfind('.'); position != std::string::npos) {
      const auto extension = filename.substr(position);
      if (extension == ".png") {
        _pixmappool->get(filename);
      } else if (extension == ".ogg") {
        _soundmanager->get(filename);
      }
    }
  }
}

#ifndef NDEBUG
void resourcemanager::debug() const {
  _pixmappool->debug();
  _soundmanager->debug();
  _fontfactory->debug();
}
#endif

std::shared_ptr<renderer> resourcemanager::renderer() const {
  return _renderer;
}

std::shared_ptr<pixmappool> resourcemanager::pixmappool() const {
  return _pixmappool;
}

std::shared_ptr<soundmanager> resourcemanager::soundmanager() const {
  return _soundmanager;
}

std::shared_ptr<fontfactory> resourcemanager::fontfactory() const {
  return _fontfactory;
}
