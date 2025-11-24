#pragma once

#include "common.hpp"

template<typename T>
class componentarray {
static constexpr auto size = entitystorage::size;
public:
  componentarray() {
    entitymapping.reserve(size);
		indexmapping.reserve(size);
  }

  void insert(entity e, T component) {
		auto [it, inserted] = entitymapping.emplace(e, entitymapping.size());
		assert(inserted && "Component added to same entity more than once.");

		size_t index = it->second;
		entitymapping.emplace(index, e);
		array[index] = std::move(component);
  }

  void remove(entity e) {
    auto it = entitymapping.find(e);
		assert(it != entitymapping.end() && "Removing non-existent component.");

		size_t removed_at = it->second;
		size_t last = entitymapping.size() - 1;

		array[removed_at] = std::move(array[last]);

		auto entity = indexmapping[last];
		entitymapping[entity] = removed_at;
		indexmapping[removed_at] = entity;

		entitymapping.erase(it);
		indexmapping.erase(last);
  }

private:
  std::array<T, size> array;

  std::unordered_map<entity, size_t> entitymapping;

  std::unordered_map<size_t, entity> indexmapping;
};
