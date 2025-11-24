#pragma once

#include "common.hpp"
#include "entity.hpp"

class entitymanager final {
  static constexpr auto max_entities = entitystorage::size;

  struct slot {
    entity id;
    signature sig;
    uint32_t generation;
    bool alive;
  };

public:
  entitymanager() noexcept {
    for (size_t i = 0; i < max_entities; ++i) {
      _slots[i].id = static_cast<entity>(i);
      _slots[i].sig.reset();
      _slots[i].generation = 0;
      _slots[i].alive = false;
      _available[i] = static_cast<entity>(max_entities - 1 - i);
    }
    _count = 0;
    _available_count = max_entities;
  }

  [[nodiscard]] entity create() noexcept {
    assert(_available_count > 0 && "No more entities available.");
    
    auto const index = _available[--_available_count];
    auto& slot = _slots[index];
    
    slot.alive = true;
    slot.sig.reset();
    ++_count;
    
    return slot.id | (static_cast<entity>(slot.generation) << 32);
  }

  void destroy(entity e) noexcept {
    auto const index = _to_index(e);
    assert(index < max_entities && "Entity index out of bounds.");
    
    auto& slot = _slots[index];
    assert(slot.alive && "Destroying dead entity.");
    assert(_generation(e) == slot.generation && "Entity generation mismatch.");
    
    slot.alive = false;
    slot.sig.reset();
    ++slot.generation;
    --_count;
    
    _available[_available_count++] = index;
  }

  [[nodiscard]] bool alive(entity e) const noexcept {
    auto const index = _to_index(e);
    if (index >= max_entities) return false;
    
    auto const& slot = _slots[index];
    return slot.alive && _generation(e) == slot.generation;
  }

  void set_signature(entity e, signature sig) noexcept {
    auto const index = _to_index(e);
    assert(index < max_entities && "Entity index out of bounds.");
    assert(_slots[index].alive && "Setting signature on dead entity.");
    
    _slots[index].sig = sig;
  }

  [[nodiscard]] signature get_signature(entity e) const noexcept {
    auto const index = _to_index(e);
    assert(index < max_entities && "Entity index out of bounds.");
    assert(_slots[index].alive && "Getting signature from dead entity.");
    
    return _slots[index].sig;
  }

  [[nodiscard]] size_t count() const noexcept {
    return _count;
  }

  [[nodiscard]] size_t available() const noexcept {
    return _available_count;
  }

  [[nodiscard]] bool full() const noexcept {
    return _available_count == 0;
  }

  [[nodiscard]] bool empty() const noexcept {
    return _count == 0;
  }

  void clear() noexcept {
    for (size_t i = 0; i < max_entities; ++i) {
      _slots[i].alive = false;
      _slots[i].sig.reset();
      _slots[i].generation = 0;
      _available[i] = static_cast<entity>(max_entities - 1 - i);
    }
    _count = 0;
    _available_count = max_entities;
  }

  template<typename F>
  void each(F&& fn) const noexcept {
    for (size_t i = 0; i < max_entities; ++i) {
      if (_slots[i].alive) {
        auto const e = _slots[i].id | (static_cast<entity>(_slots[i].generation) << 32);
        fn(e, _slots[i].sig);
      }
    }
  }

  template<typename F>
  void matching(const signature& required, F&& fn) const noexcept {
    for (size_t i = 0; i < max_entities; ++i) {
      if (_slots[i].alive && signature_ops::matches(_slots[i].sig, required)) {
        auto const e = _slots[i].id | (static_cast<entity>(_slots[i].generation) << 32);
        fn(e);
      }
    }
  }

private:
  [[nodiscard]] static constexpr size_t _to_index(entity e) noexcept {
    return static_cast<size_t>(e & 0xFFFFFFFF);
  }

  [[nodiscard]] static constexpr uint32_t _generation(entity e) noexcept {
    return static_cast<uint32_t>(e >> 32);
  }

  alignas(64) slot _slots[max_entities];
  alignas(64) entity _available[max_entities];
  size_t _count;
  size_t _available_count;
};