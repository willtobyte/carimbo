#include "soundmanager.hpp"

#include "soundfx.hpp"

soundmanager::soundmanager(std::string_view scenename)
    : _scenename(scenename) {
  _sounds.reserve(8);
}

soundmanager::~soundmanager() noexcept {
  stop();
}

void soundmanager::add(std::string_view name) {
  const auto path = std::format("blobs/{}/{}.ogg", _scenename, name);
  _sounds.emplace(name, std::make_shared<soundfx>(path));
}

void soundmanager::update(float delta) {
  for (auto& [_, sound] : _sounds) {
    sound->update(delta);
  }
}

void soundmanager::populate(sol::table& pool) const {
  for (const auto& [name, sound] : _sounds) {
    assert(!pool[name].valid() && "duplicate key in pool");
    pool[name] = sound;
  }
}

void soundmanager::stop() const noexcept {
  for (const auto& [_, sound] : _sounds) {
    sound->stop();
  }
}

void soundmanager::clear() {
  _sounds.clear();
}
