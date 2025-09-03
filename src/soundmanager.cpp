#include "soundmanager.hpp"

using namespace audio;

soundmanager::soundmanager(std::shared_ptr<audiodevice> audiodevice) noexcept
    : _audiodevice(std::move(audiodevice)) {}

std::shared_ptr<soundfx> soundmanager::get(const std::string& name) noexcept {
  auto [it, inserted] = _pool.try_emplace(name);
  if (!inserted) [[unlikely]] {
    return it->second;
  }

  std::println("[soundmanager] cache miss {}", name);
  assert(_audiodevice);

  // _loop();

  it->second = std::make_shared<soundfx>(name, _effect == retro);
  return it->second;
}

void soundmanager::play(const std::string& filename, bool loop) noexcept {
  if (const auto& sound = get(std::format("blobs/{}.ogg", filename)); sound) {
    sound->play(loop);
  }
}

void soundmanager::stop(const std::string& filename) noexcept {
  if (const auto& sound = get(std::format("blobs/{}.ogg", filename)); sound) {
    sound->stop();
  }
}

void soundmanager::flush() noexcept {
  std::println("[soundmanager] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](const auto& pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  std::println("[soundmanager] {} objects have been flushed", count);
}

void soundmanager::set_loop(std::function<void()> fn) noexcept {
  _loop = std::move(fn);;
}

void soundmanager::set_effect(soundeffect effect) noexcept {
  _effect = effect;
}

#ifdef DEBUG
void soundmanager::debug() const noexcept {
  std::println("soundmanager::debug total objects: {}", _pool.size());

  for (const auto& [key, ptr] : _pool) {
    std::println("  [{}] use_count={}", key, ptr.use_count());
  }
}
#endif
