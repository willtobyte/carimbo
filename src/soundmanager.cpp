#include "soundmanager.hpp"

#include "audiodevice.hpp"
#include "io.hpp"
#include "soundfx.hpp"

using namespace audio;

soundmanager::soundmanager(std::shared_ptr<audiodevice> audiodevice)
    : _audiodevice(std::move(audiodevice)) {}

std::shared_ptr<soundfx> soundmanager::get(std::string_view filename) {
  const auto [it, inserted] = _pool.try_emplace(std::string{filename});
  if (inserted) [[likely]] {
    std::println("[soundmanager] cache miss {}", filename);
    it->second = std::make_shared<soundfx>(filename);
  }

  return it->second;
}

void soundmanager::play(std::string_view filename, bool loop) {
  if (const auto sound = get(std::format("blobs/{}.ogg", filename)); sound) {
    sound->play(loop);
  }
}

void soundmanager::stop(std::string_view filename) {
  if (const auto sound = get(std::format("blobs/{}.ogg", filename)); sound) {
    sound->stop();
  }
}

void soundmanager::flush() {
  std::println("[soundmanager] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](const auto& pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  std::println("[soundmanager] {} objects have been flushed", count);
}

void soundmanager::update(float delta) {
  for (auto& entry : _pool) {
    const auto& e = entry.second;

    if (!e) [[unlikely]] {
      continue;
    }

    if (e.use_count() <= MINIMAL_USE_COUNT) {
      continue;
    }

    e->update(delta);
  }
}

#ifndef NDEBUG
void soundmanager::debug() const {
  std::println("[soundmanager.debug] total objects: {}", _pool.size());

  for (const auto& [key, ptr] : _pool) {
    std::println("  {} use_count={}", key, ptr.use_count());
  }
}
#endif
