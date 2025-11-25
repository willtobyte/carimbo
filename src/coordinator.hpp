#pragma once

#include "componentmanager.hpp"
#include "entity.hpp"
#include "entitymanager.hpp"
#include "systemmanager.hpp"

template<typename ComponentManager, typename SystemManager>
class coordinator final {
public:
  coordinator() = default;

  [[nodiscard]] entity create_entity();

  void destroy_entity(const entity id);

  template<typename T>
  void add_component(const entity id, const T& component);

  template<typename T>
  void remove_component(const entity id);

  template<typename T>
  [[nodiscard]] T& get_component(const entity id);

  template<typename T>
  [[nodiscard]] componenttype component_type();

  template<typename T>
  [[nodiscard]] T& enroll();

  template<typename T>
  void set_system_signature(const signature signature);

private:
  ComponentManager _componentmanager{};
  entitymanager _entitymanager{};
  SystemManager _systemmanager{};

  template<typename T>
  void update_signature_bit(const entity id, const bool value);
};

template<typename ComponentManager, typename SystemManager>
entity coordinator<ComponentManager, SystemManager>::create_entity() {
  return _entitymanager.create();
}

template<typename ComponentManager, typename SystemManager>
void coordinator<ComponentManager, SystemManager>::destroy_entity(const entity id) {
  _entitymanager.destroy(id);
  _componentmanager.destroy_entity(id);
  _systemmanager.destroy_entity(id);
}

template<typename ComponentManager, typename SystemManager>
template<typename T>
void coordinator<ComponentManager, SystemManager>::add_component(const entity id, const T& component) {
  _componentmanager.template add<T>(id, component);
  update_signature_bit<T>(id, true);
}

template<typename ComponentManager, typename SystemManager>
template<typename T>
void coordinator<ComponentManager, SystemManager>::remove_component(const entity id) {
  _componentmanager.template remove<T>(id);
  update_signature_bit<T>(id, false);
}

template<typename ComponentManager, typename SystemManager>
template<typename T>
T& coordinator<ComponentManager, SystemManager>::get_component(const entity id) {
  return _componentmanager.template get<T>(id);
}

template<typename ComponentManager, typename SystemManager>
template<typename T>
componenttype coordinator<ComponentManager, SystemManager>::component_type() {
  return _componentmanager.template component_type<T>();
}

template<typename ComponentManager, typename SystemManager>
template<typename T>
T& coordinator<ComponentManager, SystemManager>::enroll() {
  return _systemmanager.template enroll<T>();
}

template<typename ComponentManager, typename SystemManager>
template<typename T>
void coordinator<ComponentManager, SystemManager>::set_system_signature(const signature signature) {
  _systemmanager.template set_signature<T>(signature);
}

template<typename ComponentManager, typename SystemManager>
template<typename T>
void coordinator<ComponentManager, SystemManager>::update_signature_bit(const entity id, const bool value) {
  auto signature = _entitymanager.signature(id);
  signature.set(_componentmanager.template component_type<T>(), value);
  _entitymanager.set_signature(id, signature);
  _systemmanager.entity_signature_changed(id, signature);
}
