#pragma once

#include "common.hpp"
#include "entity.hpp"

class system {
public:
  system() noexcept {
    _count = 0;
  }

  virtual ~system() = default;

  system(const system&) = delete;
  system& operator=(const system&) = delete;
  system(system&&) = delete;
  system& operator=(system&&) = delete;

  virtual void update(float delta) noexcept = 0;

  void set_signature(signature sig) noexcept {
    _signature = sig;
  }

  [[nodiscard]] const signature& get_signature() const noexcept {
    return _signature;
  }

  void add_entity(entity e) noexcept {
    assert(_count < entitystorage::size && "System entity list full.");
    assert(!has_entity(e) && "Entity already in system.");
    
    _entities[_count++] = e;
  }

  void remove_entity(entity e) noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (_entities[i] == e) {
        _entities[i] = _entities[--_count];
        return;
      }
    }
    assert(false && "Entity not found in system.");
  }

  [[nodiscard]] bool has_entity(entity e) const noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (_entities[i] == e) {
        return true;
      }
    }
    return false;
  }

  void clear_entities() noexcept {
    _count = 0;
  }

  [[nodiscard]] size_t count() const noexcept {
    return _count;
  }

  [[nodiscard]] bool empty() const noexcept {
    return _count == 0;
  }

  template<typename F>
  void each(F&& fn) const noexcept {
    for (size_t i = 0; i < _count; ++i) {
      fn(_entities[i]);
    }
  }

protected:
  [[nodiscard]] entity entity_at(size_t index) const noexcept {
    assert(index < _count && "Entity index out of bounds.");
    return _entities[index];
  }

  [[nodiscard]] entity* begin() noexcept { return _entities; }
  [[nodiscard]] entity* end() noexcept { return _entities + _count; }
  [[nodiscard]] const entity* begin() const noexcept { return _entities; }
  [[nodiscard]] const entity* end() const noexcept { return _entities + _count; }

private:
  alignas(64) entity _entities[entitystorage::size];
  signature _signature;
  size_t _count;
};