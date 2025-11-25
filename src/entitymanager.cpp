#include "entitymanager.hpp"

entitymanager::entitymanager() noexcept
    : entities(kEntityCapacity) {
  std::iota(entities.begin(), entities.end(), entity{0});
}

entity entitymanager::create() noexcept {
  assert(!entities.empty() && "too many entities in existence.");

  const auto id = entities.back();
  entities.pop_back();
  signatures[id].reset();
  return id;
}

void entitymanager::destroy(const entity id) noexcept {
  assert(id >= entity{0});
  assert(id < signatures.size() && "entity out of range.");

  signatures[id].reset();
  entities.emplace_back(id);
}

void entitymanager::set_signature(const entity id, const ::signature signature) noexcept {
  assert(id < signatures.size() && "entity out of range.");

  signatures[id] = signature;
}

signature entitymanager::signature(const entity id) const noexcept {
  assert(id < signatures.size() && "entity out of range.");

  return signatures[id];
}
