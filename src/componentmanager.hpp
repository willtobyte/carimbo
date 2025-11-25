#pragma once

#include "component.hpp"
#include "componentarray.hpp"
#include "entity.hpp"

template<typename T, typename... Ts>
struct type_index;

template<typename T, typename Head, typename... Tail>
struct type_index<T, Head, Tail...> {
  static constexpr std::size_t value =
    std::is_same_v<T, Head> ? 0u : 1u + type_index<T, Tail...>::value;
};

template<typename T>
struct type_index<T> {
  static_assert(!std::is_same_v<T, T>, "type not found in componentmanager pack.");
};

template<typename T, typename... Ts>
inline constexpr std::size_t type_index_v = type_index<T, Ts...>::value;

template<typename... Components>
class componentmanager final {
public:
  componentmanager() noexcept = default;

  template<typename T>
  static constexpr componenttype component_type() noexcept;

  template<typename T>
  void add(const entity id, const T& c) noexcept;

  template<typename T>
  void remove(const entity id) noexcept;

  template<typename T>
  [[nodiscard]] T& get(const entity id) noexcept;

  template<typename T>
  [[nodiscard]] const T& get(const entity id) const noexcept;

  void destroy_entity(const entity id) noexcept;

private:
  std::tuple<componentarray<Components>...> _arrays{};

  template<typename T>
  [[nodiscard]] componentarray<T>& array() noexcept;

  template<typename T>
  [[nodiscard]] const componentarray<T>& array() const noexcept;

  template<std::size_t... Is>
  void purge(const entity id, std::index_sequence<Is...>) noexcept;
};

template<typename... Components>
template<typename T>
constexpr componenttype componentmanager<Components...>::component_type() noexcept {
  static_assert((std::is_same_v<T, Components> || ...),
                "Component not included in this manager.");
  constexpr auto i = type_index_v<T, Components...>;
  return static_cast<componenttype>(i);
}

template<typename... Components>
template<typename T>
void componentmanager<Components...>::add(const entity id, const T& c) noexcept {
  array<T>().insert(id, c);
}

template<typename... Components>
template<typename T>
void componentmanager<Components...>::remove(const entity id) noexcept {
  array<T>().remove(id);
}

template<typename... Components>
template<typename T>
T& componentmanager<Components...>::get(const entity id) noexcept {
  return array<T>().get(id);
}

template<typename... Components>
template<typename T>
const T& componentmanager<Components...>::get(const entity id) const noexcept {
  return array<T>().get(id);
}

template<typename... Components>
void componentmanager<Components...>::destroy_entity(const entity id) noexcept {
  purge(id, std::index_sequence_for<Components...>{});
}

template<typename... Components>
template<typename T>
componentarray<T>& componentmanager<Components...>::array() noexcept {
  constexpr auto i = type_index_v<T, Components...>;
  return std::get<i>(_arrays);
}

template<typename... Components>
template<typename T>
const componentarray<T>& componentmanager<Components...>::array() const noexcept {
  constexpr auto i = type_index_v<T, Components...>;
  return std::get<i>(_arrays);
}

template<typename... Components>
template<std::size_t... Is>
void componentmanager<Components...>::purge(const entity id, std::index_sequence<Is...>) noexcept {
  (std::get<Is>(_arrays).destroy_entity(id), ...);
}
