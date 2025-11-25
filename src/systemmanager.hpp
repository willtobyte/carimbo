#pragma once

#include "common.hpp"

#include "component.hpp"
#include "componentmanager.hpp"
#include "entity.hpp"

class system {
public:
  std::unordered_set<entity> entities;
};

template<typename... systems>
class systemmanager final {
public:
  systemmanager() noexcept = default;

  template<typename T>
  T& enroll() noexcept;

  template<typename T>
  void set_signature(const signature& signature) noexcept;

  void destroy_entity(const entity id) noexcept;

  void entity_signature_changed(const entity id, const signature& signature) noexcept;

  template<typename T>
  [[nodiscard]] T& system() noexcept;

  template<typename T>
  [[nodiscard]] const T& system() const noexcept;

private:
  std::tuple<systems...> _systems;
  std::array<signature, sizeof...(systems)> _signatures;

  template<std::size_t... Is>
  void sweep(const entity id, std::index_sequence<Is...>) noexcept;

  template<std::size_t I>
  void update_one(const entity id, const signature& signature) noexcept;

  template<std::size_t... Is>
  void update(const entity id, const signature& signature, std::index_sequence<Is...>) noexcept;
};

template<typename... systems>
template<typename T>
T& systemmanager<systems...>::enroll() noexcept {
  static_assert((std::is_same_v<T, systems> || ...),
                "system not part of this system_manager.");
  return system<T>();
}

template<typename... systems>
template<typename T>
void systemmanager<systems...>::set_signature(const signature& signature) noexcept {
  constexpr auto i = type_index_v<T, systems...>;
  static_assert((std::is_same_v<T, systems> || ...),
                "system not part of this system_manager.");
  _signatures[i] = signature;
}

template<typename... systems>
void systemmanager<systems...>::destroy_entity(const entity id) noexcept {
  sweep(id, std::index_sequence_for<systems...>{});
}

template<typename... systems>
void systemmanager<systems...>::entity_signature_changed(const entity id, const signature& signature) noexcept {
  update(id, signature, std::index_sequence_for<systems...>{});
}

template<typename... systems>
template<typename T>
T& systemmanager<systems...>::system() noexcept {
  constexpr auto i = type_index_v<T, systems...>;
  return std::get<i>(_systems);
}

template<typename... systems>
template<typename T>
const T& systemmanager<systems...>::system() const noexcept {
  constexpr auto i = type_index_v<T, systems...>;
  return std::get<i>(_systems);
}

template<typename... systems>
template<std::size_t... Is>
void systemmanager<systems...>::sweep(const entity id, std::index_sequence<Is...>) noexcept {
  (std::get<Is>(_systems).entities.erase(id), ...);
}

template<typename... systems>
template<std::size_t I>
void systemmanager<systems...>::update_one(const entity id, const signature& signature) noexcept {
  auto& system = std::get<I>(_systems);

  if ((signature & _signatures[I]) == _signatures[I]) {
    system.entities.insert(id);
  } else {
    system.entities.erase(id);
  }
}

template<typename... systems>
template<std::size_t... Is>
void systemmanager<systems...>::update(const entity id, const signature& signature, std::index_sequence<Is...>) noexcept {
  (update_one<Is>(id, signature), ...);
}
