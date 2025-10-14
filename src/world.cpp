#include "world.hpp"

#include "object.hpp"

using namespace framework;

static inline bool owner_equal(const std::weak_ptr<object>& a, const std::shared_ptr<object>& b) noexcept {
  const auto& cmp = std::owner_less<void>{};
  if (cmp(a, b)) return false;
  if (cmp(b, a)) return false;
  return true;
}

void world::add(const std::shared_ptr<object>& object) {
  if (!object) [[unlikely]] {
    return;
  }

  _objects.emplace_back(object);
}

void world::remove(const std::shared_ptr<object>& object) {
  if (!object) [[unlikely]] {
    return;
  }

  std::erase_if(_objects, [&](const std::weak_ptr<framework::object>& ptr) noexcept {
    return owner_equal(ptr, object);
  });
}

void world::update(float delta) noexcept {
  std::erase_if(_objects, [](const std::weak_ptr<framework::object>& ptr) noexcept {
    return ptr.expired();
  });

  for (const auto& o : _objects) {
    const auto ptr = o.lock();
    if (!ptr) [[unlikely]] {
      continue;
    }

    ptr->update(delta);
  }
}

void world::draw() const noexcept {
  for (const auto& o : _objects) {
    const auto ptr = o.lock();
    if (!ptr) [[unlikely]] {
      continue;
    }

    ptr->draw();
  }
}
