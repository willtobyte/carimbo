#pragma once

#include "common.hpp"
#include "entity.hpp"

template<typename T>
class componentarray final {
  static constexpr auto max_entities = entitystorage::size;
  static constexpr auto invalid_index = std::numeric_limits<size_t>::max();

public:
  constexpr componentarray() noexcept : _size(0) {
    for (size_t i = 0; i < max_entities; ++i) {
      _lookup[i] = invalid_index;
    }
  }

  void insert(entity e, T component) noexcept {
    auto const index = e & 0xFFFFFFFF;
    assert(index < max_entities && "Entity ID out of bounds.");
    assert(_lookup[index] == invalid_index && "Component added to same entity more than once.");
    assert(_size < max_entities && "Component array full.");

    _lookup[index] = _size;
    _entities[_size] = e;
    _components[_size] = component;
    ++_size;
  }

  void remove(entity e) noexcept {
    auto const index = e & 0xFFFFFFFF;
    assert(index < max_entities && "Entity ID out of bounds.");
    assert(_lookup[index] != invalid_index && "Removing non-existent component.");
    assert(_size > 0 && "Removing from empty array.");

    auto const removed_index = _lookup[index];
    auto const last_index = _size - 1;

    if (removed_index != last_index) {
      auto const last_entity = _entities[last_index];
      auto const last_entity_index = last_entity & 0xFFFFFFFF;

      _components[removed_index] = _components[last_index];
      _entities[removed_index] = last_entity;
      _lookup[last_entity_index] = removed_index;
    }

    _lookup[index] = invalid_index;
    --_size;
  }

  [[nodiscard]] T& get(entity e) noexcept {
    auto const index = e & 0xFFFFFFFF;
    assert(index < max_entities && "Entity ID out of bounds.");
    assert(_lookup[index] != invalid_index && "Getting non-existent component.");
    return _components[_lookup[index]];
  }

  [[nodiscard]] const T& get(entity e) const noexcept {
    auto const index = e & 0xFFFFFFFF;
    assert(index < max_entities && "Entity ID out of bounds.");
    assert(_lookup[index] != invalid_index && "Getting non-existent component.");
    return _components[_lookup[index]];
  }

  [[nodiscard]] bool has(entity e) const noexcept {
    auto const index = e & 0xFFFFFFFF;
    return index < max_entities && _lookup[index] != invalid_index;
  }

  [[nodiscard]] size_t size() const noexcept {
    return _size;
  }

  [[nodiscard]] bool empty() const noexcept {
    return _size == 0;
  }

  [[nodiscard]] bool full() const noexcept {
    return _size == max_entities;
  }

  void clear() noexcept {
    for (size_t i = 0; i < _size; ++i) {
      auto const index = _entities[i] & 0xFFFFFFFF;
      _lookup[index] = invalid_index;
    }
    _size = 0;
  }

  [[nodiscard]] T* begin() noexcept { return _components; }
  [[nodiscard]] T* end() noexcept { return _components + _size; }
  [[nodiscard]] const T* begin() const noexcept { return _components; }
  [[nodiscard]] const T* end() const noexcept { return _components + _size; }
  [[nodiscard]] const T* cbegin() const noexcept { return _components; }
  [[nodiscard]] const T* cend() const noexcept { return _components + _size; }

  [[nodiscard]] T& at_index(size_t idx) noexcept {
    assert(idx < _size && "Index out of bounds.");
    return _components[idx];
  }

  [[nodiscard]] const T& at_index(size_t idx) const noexcept {
    assert(idx < _size && "Index out of bounds.");
    return _components[idx];
  }

  [[nodiscard]] entity entity_at_index(size_t idx) const noexcept {
    assert(idx < _size && "Index out of bounds.");
    return _entities[idx];
  }

  template<typename F>
  void each(F&& fn) noexcept {
    for (size_t i = 0; i < _size; ++i) {
      fn(_entities[i], _components[i]);
    }
  }

  template<typename F>
  void each(F&& fn) const noexcept {
    for (size_t i = 0; i < _size; ++i) {
      fn(_entities[i], _components[i]);
    }
  }

private:
  alignas(64) T _components[max_entities];
  alignas(64) entity _entities[max_entities];
  alignas(64) size_t _lookup[max_entities];
  size_t _size;
};