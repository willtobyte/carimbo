#include "world.hpp"

#include "object.hpp"

using namespace framework;

static box_t to_box(const geometry::rectangle& r) noexcept {
  return box_t(point_t(r.x(), r.y()), point_t(r.x() + r.width(), r.y() + r.height()));
}

world::world() noexcept {
  _index.reserve(64);
  _aabbs.reserve(64);
}

void world::add(const std::shared_ptr<object>& object) {
  if (!object) [[unlikely]] {
    return;
  }

  const auto id = object->id();

  _index.insert_or_assign(id, std::weak_ptr<framework::object>(object));
}

void world::remove(const std::shared_ptr<object>& object) {
  if (!object) [[unlikely]] {
    return;
  }

  const auto id = object->id();

  if (const auto it = _aabbs.find(id); it != _aabbs.end()) {
    _spatial.remove(std::make_pair(it->second, id));
    _aabbs.erase(it);
  }

  _index.erase(id);
}

void world::update(float delta) noexcept {
  for (auto it = _index.begin(); it != _index.end(); ) {
    auto object = it->second.lock();
    if (!object) [[unlikely]] {
      const auto id = it->first;
      if (const auto it = _aabbs.find(id); it != _aabbs.end()) {
        _spatial.remove(std::make_pair(it->second, id));
        _aabbs.erase(it);
      }

      it = _index.erase(it);
      continue;
    }

    if (!object->dirty()) [[unlikely]] {
      ++it;
      continue;
    }

    const auto boundingbox_opt = object->boundingbox();
    if (!boundingbox_opt) [[unlikely]] {
      ++it;
      continue;
    }

    const auto id  = object->id();
    const auto aabb = to_box(*boundingbox_opt);
    if (auto it = _aabbs.find(id); it != _aabbs.end()) {
      _spatial.remove(std::make_pair(it->second, id));
    }

    _spatial.insert(std::make_pair(aabb, id));
    _aabbs.insert_or_assign(id, aabb);

    ++it;
  }

  // query entitis colliding

}

void world::draw() const noexcept {
#ifdef DEBUG
#endif
}
