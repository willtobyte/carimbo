#include "world.hpp"

#include "object.hpp"

using namespace framework;

static box_t to_box(const geometry::rectangle& r) noexcept {
  return box_t(point_t(r.x(), r.y()), point_t(r.x() + r.width(), r.y() + r.height()));
}

template <class Pred>
static inline std::vector<std::weak_ptr<object>>
rtree_query_to_vector(bgi::rtree<std::pair<box_t, uint64_t>, bgi::rstar<16>>& spatial,
                   Pred&& pred,
                   std::vector<std::pair<box_t, uint64_t>>& hits,
                   const std::unordered_map<uint64_t, std::weak_ptr<object>>& index) {
  hits.clear();
  spatial.query(std::forward<Pred>(pred), std::back_inserter(hits));

  std::vector<std::weak_ptr<object>> out;
  if (hits.empty()) return out;

  out.reserve(hits.size());
  for (const auto& h : hits) {
    const auto it = index.find(h.second);
    if (it == index.end()) continue;
    if (it->second.expired()) continue;
    out.emplace_back(it->second);
  }

  return out;
}

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
  _hits.reserve(16);
  _emitted.reserve(16);
}

void world::add(const std::shared_ptr<object>& object) {
  if (!object) [[unlikely]] {
    return;
  }

  const auto id = object->id();

  _index.insert_or_assign(id, std::weak_ptr<framework::object>(object));

  const auto boundingbox_opt = object->boundingbox();
  if (!boundingbox_opt) [[unlikely]] {
    return;
  }

  const auto aabb = to_box(*boundingbox_opt);
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

std::vector<std::weak_ptr<object>> world::query(float x, float y) {
  const point_t p{x, y};
  return rtree_query_to_vector(_spatial, bgi::intersects(p), _hits, _index);
}

std::vector<std::weak_ptr<object>> world::query(float x, float y, float w, float h) {
  const auto x2 = x + w;
  const auto y2 = y + h;

  const point_t minp(std::min(x, x2), std::min(y, y2));
  const point_t maxp(std::max(x, x2), std::max(y, y2));
  const box_t area(minp, maxp);

  return rtree_query_to_vector(_spatial, bgi::intersects(area), _hits, _index);
}

void world::update(float delta) noexcept {
  _dirties.reserve(_index.size());
  _dirties.clear();

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

    const auto boundingbox_opt = object->boundingbox();
    if (!boundingbox_opt) [[unlikely]] {
      ++it;
      continue;
    }

    if (!object->dirty()) [[unlikely]] {
      ++it;
      continue;
    }

    std::println("[world] object {} with id {} is dirty", object->kind(), object->id());

    const auto id  = object->id();
    const auto aabb = to_box(*boundingbox_opt);
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

    _pairs.clear();
    _hits.clear();
    _spatial.query(bgi::intersects(aabb), std::back_inserter(_hits));

    for (const auto& hit : _hits) {
      const auto other = hit.second;
      const auto aid = std::min(id, other);
      const auto bid = std::max(id, other);
      if (!_pairs.insert({aid, bid}).second) {
        continue;
      }

      const auto aptr = _index.at(id);
      const auto a = aptr.lock();
      if (!a) [[unlikely]] continue;

      const auto bptr = _index.at(other);
      const auto b = bptr.lock();
      if (!b) [[unlikely]] continue;

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
