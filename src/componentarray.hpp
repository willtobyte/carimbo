#pragma once

#include "entity.hpp"

template<typename T>
class component_array final {
public:
  component_array() noexcept;

  void insert(const entity id, const T& component) noexcept;

  void remove(const entity id) noexcept;

  [[nodiscard]] T& get(const entity id) noexcept;

  [[nodiscard]] const T& get(const entity id) const noexcept;

  void entity_destroyed(const entity id) noexcept;

  [[nodiscard]] bool has(const entity id) const noexcept;

private:
  static constexpr auto __invalid = std::numeric_limits<std::size_t>::max();

  std::size_t _size{0};
  std::array<T, entity_n> _components;
  std::array<entity, entity_n> _index_to_entity;
  std::array<std::size_t, entity_n> _entity_to_index;
};

template<typename T>
component_array<T>::component_array() noexcept {
  _entity_to_index.fill(__invalid);
}

template<typename T>
void component_array<T>::insert(const entity id, const T& component) noexcept {
  assert(id < entity_n);
  assert(!has(id) && "component added to same entity more than once.");

  _components[_size] = component;
  _index_to_entity[_size] = id;
  _entity_to_index[id] = _size;
  ++_size;
}

template<typename T>
void component_array<T>::remove(const entity id) noexcept {
  assert(has(id) && "removing non-existent component.");

  const auto remove_index = _entity_to_index[id];
  const auto last_index = _size - 1;
  const auto last_entity = _index_to_entity[last_index];

  _components[remove_index] = _components[last_index];
  _index_to_entity[remove_index] = last_entity;
  _entity_to_index[last_entity] = remove_index;
  _entity_to_index[id] = __invalid;

  --_size;
}

template<typename T>
T& component_array<T>::get(const entity id) noexcept {
  assert(has(id) && "retrieving non-existent component.");
  return _components[_entity_to_index[id]];
}

template<typename T>
const T& component_array<T>::get(const entity id) const noexcept {
  assert(has(id) && "retrieving non-existent component.");
  return _components[_entity_to_index[id]];
}

template<typename T>
void component_array<T>::entity_destroyed(const entity id) noexcept {
  if (!has(id)) [[unlikely]] return;
  remove(id);
}

template<typename T>
bool component_array<T>::has(const entity id) const noexcept {
  if (id >= entity_n) [[unlikely]] return false;
  return _entity_to_index[id] != __invalid;
}
