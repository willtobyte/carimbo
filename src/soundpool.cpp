#include "soundpool.hpp"

#include "soundfx.hpp"

soundpool::soundpool(std::string_view scenename)
    : _scenename(scenename) {
  _sounds.reserve(8);
}

soundpool::~soundpool() noexcept {
  stop();
}

void soundpool::add(std::string_view name) {
  auto [it, inserted] = _sounds.try_emplace(name);
  if (inserted) {
    it->second = std::make_shared<soundfx>(std::format("blobs/{}/{}.opus", _scenename, name));
  }
}

void soundpool::update(float delta) {
  for (auto& [_, sound] : _sounds) {
    sound->update(delta);
  }
}

void soundpool::populate(sol::table& pool) const {
  for (const auto& [name, sound] : _sounds) {
    assert(!pool[name].valid() && "duplicate key in pool");
    pool[name] = sound;
  }
}

void soundpool::stop() const noexcept {
  for (const auto& [_, sound] : _sounds) {
    sound->stop();
  }
}

void soundpool::clear() {
  _sounds.clear();
}
