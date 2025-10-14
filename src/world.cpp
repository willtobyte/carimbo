#include "world.hpp"
#include "object.hpp"

using namespace framework;

world::world() noexcept {
  _index.reserve(64);
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

  _index.erase(id);
}

void world::update(float delta) noexcept {
  std::erase_if(_index, [&](auto& it) noexcept {
    auto ptr = it.second.lock();
    if (!ptr) {
      return true;
    }

    const auto dirty = ptr->dirty();
    if (dirty) {
      // update the tree
    }

    return false;
  });

  // SDL_Event event{};
  // event.type = static_cast<uint32_t>(type::collision);
  // event.user.data1 = _envelopepool->acquire(collisionenvelope(a->id(), b->id())).release();
  // SDL_PushEvent(&event);
}

void world::draw() const noexcept {
#ifdef DEBUG
#endif
}
