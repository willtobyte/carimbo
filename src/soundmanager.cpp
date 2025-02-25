#include "soundmanager.hpp"

using namespace audio;

soundmanager::soundmanager(const std::shared_ptr<audiodevice> audiodevice) noexcept
    : _audiodevice(std::move(audiodevice)) {}

std::shared_ptr<soundfx> soundmanager::get(const std::string &filename) noexcept {
  if (auto it = _pool.find(filename); it != _pool.end()) [[likely]] {
    return it->second;
  }

  std::cout << "[soundmanager] cache miss " << filename << std::endl;

  assert(_audiodevice);

  auto ptr = std::make_shared<soundfx>(_audiodevice, filename);
  _pool.emplace(filename, ptr);

  return ptr;
}

void soundmanager::play(const std::string &filename) noexcept {
  if (const auto &sound = get("blobs/" + filename + ".ogg"); sound) {
    sound->play();
  }
}

void soundmanager::stop(const std::string &filename) noexcept {
  if (const auto &sound = get(filename); sound) {
    sound->stop();
  }
}

void soundmanager::flush() noexcept {
  const auto count = _pool.size();
  std::cout << "[soundmanager] actual size " << count << std::endl;
  _pool.clear();
  std::cout << "[soundmanager] " << count << " objects have been flushed" << std::endl;
}
