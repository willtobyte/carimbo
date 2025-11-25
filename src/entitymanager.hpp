#pragma once

#include "common.hpp"

#include "entity.hpp"

class entitymanager final {
public:
  entitymanager() noexcept
      : entities(entity_n) {
    std::iota(entities.begin(), entities.end(), entity{0});
  }

  [[nodiscard]] entity create() noexcept {
    assert(!entities.empty() && "too many entities in existence.");

    const auto id = entities.back();
		entities.pop_back();
		signatures[id].reset();
		return id;
  }

  void destroy(const entity id) noexcept {
    assert(id >= entity{0});
    assert(static_cast<std::size_t>(id) < signatures.size() && "entity out of range.");

    signatures[id].reset();
    entities.emplace_back(id);
  }

  void set_signature(const entity id, const signature sig) noexcept {
    assert(id < signatures.size() && "entity out of range.");

    signatures[static_cast<std::size_t>(id)] = sig;
  }

  [[nodiscard]] signature get_signature(const entity id) const noexcept {
    assert(id < signatures.size() && "entity out of range.");

    return signatures[static_cast<std::size_t>(id)];
  }

private:
  std::array<signature, entity_n> signatures;
  std::vector<entity> entities;
};
