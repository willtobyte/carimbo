#include "world.hpp"

#include "object.hpp"

using namespace framework;

template <class Map>
static inline const typename Map::mapped_type* find_ptr(const Map& m,
                                                        const typename Map::key_type& k) noexcept {
  const auto it = m.find(k);
  if (it == m.end()) return nullptr;
  return std::addressof(it->second);
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

  const auto aabb = to_box(object->boundingbox());
  _spatial.insert(std::make_pair(aabb, id));
  _aabbs.insert_or_assign(id, aabb);
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
  _dirties.reserve(_index.size());
  _dirties.clear();
  _hits.reserve(_index.size());
  _pairs.reserve(_index.size());
  _pairs.clear();

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

    std::println("[world] object {} with id {} is dirty", object->kind(), object->id());

    const auto id  = object->id();
    const auto aabb = to_box(object->boundingbox());
    if (const auto it = _aabbs.find(id); it != _aabbs.end()) {
      _spatial.remove(std::make_pair(it->second, id));
    }

    _spatial.insert(std::make_pair(aabb, id));
    _aabbs.insert_or_assign(id, aabb);
    _dirties.emplace_back(id);

    ++it;
  }

  for (const auto id : _dirties) {
    const auto it = _aabbs.find(id);
    if (it == _aabbs.end()) [[unlikely]] {
      continue;
    }

    const auto& aabb = it->second;

    _hits.clear();
    _spatial.query(bgi::intersects(aabb), std::back_inserter(_hits));

    for (const auto& hit : _hits) {
      const auto other = hit.second;
      const auto aid = std::min(id, other);
      const auto bid = std::max(id, other);
      if (!_pairs.insert({aid, bid}).second) {
        continue;
      }

      const auto ait = _index.find(id);
      if (ait == _index.end()) continue;
      auto a = ait->second.lock();
      if (!a) continue;

      const auto bit = _index.find(other);
      if (bit == _index.end()) continue;
      auto b = bit->second.lock();
      if (!b) continue;

      if (const auto* callback = find_ptr(a->_collisionmapping, b->kind())) (*callback)(a, b);
      if (const auto* callback = find_ptr(b->_collisionmapping, a->kind())) (*callback)(b, a);

      SDL_Event event{};
      event.type = static_cast<uint32_t>(input::event::type::collision);
      event.user.data1 = _envelopepool->acquire(collisionenvelope(id, other)).release();
      SDL_PushEvent(&event);
    }
  }
}

void world::draw() const noexcept {
#ifdef DEBUG
#endif
}
