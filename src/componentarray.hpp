#pragma once

#include "common.hpp"

#include "entity.hpp"

template<typename T>
class componentarray {
  static constexpr auto max_entities = entitystorage::size;
  static constexpr auto invalid_index = std::numeric_limits<size_t>::max();

public:
  componentarray() noexcept {
    _lookup.assign(max_entities, invalid_index);
    _components.reserve(max_entities);
    _entities.reserve(max_entities);
  }

  void insert(const entity e, T component) noexcept {
    assert(e < max_entities && "Entity ID out of bounds.");
    assert(_lookup[e] == invalid_index && "Component added to same entity more than once.");

    auto const index = _components.size();
    _lookup[e] = index;
    _entities.push_back(e);
    _components.push_back(std::move(component));
  }

  void remove(const entity e) noexcept {
    assert(e < max_entities && "Entity ID out of bounds.");
    assert(_lookup[e] != invalid_index && "Removing non-existent component.");

    auto const removed_index = _lookup[e];
    auto const last_index = _components.size() - 1;

    if (removed_index != last_index) {
      auto const last_entity = _entities[last_index];

      _components[removed_index] = std::move(_components[last_index]);
      _entities[removed_index] = last_entity;
      _lookup[last_entity] = removed_index;
    }

    _components.pop_back();
    _entities.pop_back();
    _lookup[e] = invalid_index;
  }

  [[nodiscard]] T& get(const entity e) noexcept {
    assert(_lookup[e] != invalid_index && "Getting non-existent component.");
    return _components[_lookup[e]];
  }

  [[nodiscard]] const T& get(const entity e) const noexcept {
    assert(_lookup[e] != invalid_index && "Getting non-existent component.");
    return _components[_lookup[e]];
  }

  [[nodiscard]] bool has(const entity e) const noexcept {
    return e < max_entities && _lookup[e] != invalid_index;
  }

  [[nodiscard]] size_t size() const noexcept {
    return _components.size();
  }

  [[nodiscard]] bool empty() const noexcept {
    return _components.empty();
  }

  [[nodiscard]] size_t capacity() const noexcept {
    return _components.capacity();
  }

  void reserve(size_t n) {
    _components.reserve(n);
    _entities.reserve(n);
  }

  void clear() noexcept {
    _components.clear();
    _entities.clear();
    std::fill(_lookup.begin(), _lookup.end(), invalid_index);
  }

  [[nodiscard]] auto begin() noexcept { return _components.begin(); }
  [[nodiscard]] auto end() noexcept { return _components.end(); }
  [[nodiscard]] auto begin() const noexcept { return _components.begin(); }
  [[nodiscard]] auto end() const noexcept { return _components.end(); }
  [[nodiscard]] auto cbegin() const noexcept { return _components.cbegin(); }
  [[nodiscard]] auto cend() const noexcept { return _components.cend(); }

  [[nodiscard]] const std::vector<entity>& entities() const noexcept {
    return _entities;
  }

  [[nodiscard]] T& at_index(size_t index) noexcept {
    assert(index < _components.size() && "Index out of bounds.");
    return _components[index];
  }

  [[nodiscard]] const T& at_index(size_t index) const noexcept {
    assert(index < _components.size() && "Index out of bounds.");
    return _components[index];
  }

  [[nodiscard]] entity entity_at_index(size_t index) const noexcept {
    assert(index < _entities.size() && "Index out of bounds.");
    return _entities[index];
  }

private:
  std::vector<T> _components;
  std::vector<size_t> _lookup;
  std::vector<entity> _entities;
};
