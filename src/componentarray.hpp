#pragma once

#include "common.hpp"

template<typename T>
class componentarray {
  static constexpr auto max_entities = entitystorage::size;
  static constexpr auto invalid_index = std::numeric_limits<size_t>::max();

public:
  componentarray() noexcept {
    lookup.resize(max_entities, invalid_index);
    components.reserve(max_entities);
    reverse.reserve(max_entities);
  }

  void insert(const entity e, T component) noexcept {
    assert(e < max_entities && "Entity ID out of bounds.");
    assert(lookup[e] == invalid_index && "Component added to same entity more than once.");

    auto const index = components.size();
    lookup[e] = index;
    reverse.push_back(e);
    components.push_back(std::move(component));
  }

  void remove(const entity e) noexcept {
    assert(e < max_entities && "Entity ID out of bounds.");
    assert(lookup[e] != invalid_index && "Removing non-existent component.");

    auto const removed_index = lookup[e];
    auto const last_index = components.size() - 1;
    
    if (removed_index != last_index) {
      auto const last_entity = reverse[last_index];
      
      components[removed_index] = std::move(components[last_index]);
      reverse[removed_index] = last_entity;
      lookup[last_entity] = removed_index;
    }
    
    components.pop_back();
    reverse.pop_back();
    lookup[e] = invalid_index;
  }

  [[nodiscard]] T& get(const entity e) noexcept {
    assert(lookup[e] != invalid_index && "Getting non-existent component.");
    return components[lookup[e]];
  }

  [[nodiscard]] const T& get(const entity e) const noexcept {
    assert(lookup[e] != invalid_index && "Getting non-existent component.");
    return components[lookup[e]];
  }

  [[nodiscard]] bool has(const entity e) const noexcept {
    return e < max_entities && lookup[e] != invalid_index;
  }

  [[nodiscard]] size_t size() const noexcept {
    return components.size();
  }

  [[nodiscard]] auto begin() noexcept { return components.begin(); }
  [[nodiscard]] auto end() noexcept { return components.end(); }
  [[nodiscard]] auto begin() const noexcept { return components.begin(); }
  [[nodiscard]] auto end() const noexcept { return components.end(); }

private:
  std::vector<T> components;
  std::vector<size_t> lookup;
  std::vector<entity> reverse;
};