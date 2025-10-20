#include "world.hpp"

#include "object.hpp"

using namespace framework;

template <class Map>
static inline const typename Map::mapped_type* find_ptr(
  const Map& m,
  const typename Map::key_type& k
) noexcept {
  const auto it = m.find(k);
  if (it == m.end()) return nullptr;
  return std::addressof(it->second);
}

world::world(std::shared_ptr<graphics::renderer> renderer) noexcept
    : _renderer(std::move(renderer)) {
  _index.reserve(64);
  _aabbs.reserve(64);
}

void world::add(const std::shared_ptr<object>& object) {
  if (!object) [[unlikely]] {
    return;
  }

  const auto id = object->id();

  _index.insert_or_assign(id, std::weak_ptr<framework::object>(object));

  if (const auto it = _aabbs.find(id); it != _aabbs.end()) {
    _spatial.remove(std::make_pair(it->second, id));
  }

  const auto aabb_opt = object->aabb();
  if (!aabb_opt.has_value()) [[unlikely]] {
    return;
  }

  const auto aabb = to_box(*aabb_opt);
  _spatial.insert(std::make_pair(aabb, id));
  _aabbs.insert_or_assign(id, aabb);
}

void world::remove(uint64_t id) {
  if (const auto it = _aabbs.find(id); it != _aabbs.end()) {
    _spatial.remove(std::make_pair(it->second, id));
    _aabbs.erase(it);
  }

  _index.erase(id);
}

void world::update(float delta) noexcept {
  _dirties.clear();
  _dirties.reserve(_index.size());

  _hits.clear();
  _hits.reserve(_index.size());

  _pairs.clear();
  _pairs.reserve(_index.size());

  for (auto it = _index.begin(); it != _index.end(); ) {
    const auto id = it->first;
    std::weak_ptr<object> weak = it->second;
    auto object = weak.lock();

    std::optional<geometry::rectangle> aabb_opt;
    if (!object) goto remove;
    if (!object->visible()) goto remove;

    aabb_opt = object->aabb();
    if (!aabb_opt.has_value()) goto remove;
    if (!object->dirty()) {
      ++it;
      continue;
    }

#ifdef DEBUG
    std::println("[world] object {} with id {} is dirty", object->kind(), object->id());
#endif

    {
      const auto aabb = to_box(*aabb_opt);
      const auto found = _aabbs.find(id);
      if (found != _aabbs.end()) _spatial.remove({found->second, id});
      _spatial.insert({aabb, id});
      _aabbs.insert_or_assign(id, aabb);
      _dirties.emplace_back(id);
      ++it;
      continue;
    }

  remove:
    {
      const auto ait = _aabbs.find(id);
      if (ait != _aabbs.end()) {
        _spatial.remove({ait->second, id});
        _aabbs.erase(ait);
      }
      if (!object) {
        it = _index.erase(it);
        continue;
      }
      ++it;
    }
  }

  for (const auto id : _dirties) {
    const auto ait = _aabbs.find(id);
    if (ait == _aabbs.end()) [[unlikely]] continue;

    const auto& aabb = ait->second;

    const auto ai = _index.find(id);
    if (ai == _index.end()) [[unlikely]] continue;
    auto a = ai->second.lock();
    if (!a) continue;

    _hits.clear();
    _spatial.query(bgi::intersects(aabb), std::back_inserter(_hits));

    for (const auto& hit : _hits) {
      const auto other = hit.second;
      if (other == id) continue;

      if (!_pairs.emplace(std::min(id, other), std::max(id, other)).second) continue;

      const auto bi = _index.find(other);
      if (bi == _index.end()) continue;
      auto b = bi->second.lock();
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
  SDL_SetRenderDrawColor(*_renderer, 0, 255, 0, 255);
  for (const auto& kv : _aabbs) {
    const auto& aabb = kv.second;

    SDL_FRect rect;
    rect.x = aabb.min_corner().x();
    rect.y = aabb.min_corner().y();
    rect.w = aabb.max_corner().x() - rect.x;
    rect.h = aabb.max_corner().y() - rect.y;
    SDL_RenderRect(*_renderer, &rect);
  }
#endif
}
