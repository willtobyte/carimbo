#pragma once

#include "common.hpp"
#include "componentarray.hpp"
#include "entity.hpp"

class componentmanager final {
  static constexpr auto max_component_types = componentstorage::size;
  static constexpr auto component_storage_size = sizeof(componentarray<transform>) > sizeof(componentarray<physics>)
    ? sizeof(componentarray<transform>)
    : sizeof(componentarray<physics>);
  static constexpr auto component_storage_align = alignof(componentarray<transform>) > alignof(componentarray<physics>)
    ? alignof(componentarray<transform>)
    : alignof(componentarray<physics>);

  struct icomponentarray {
    virtual ~icomponentarray() = default;
    virtual void destroy(entity e) noexcept = 0;
    virtual void clear() noexcept = 0;
  };

  template<typename T>
  struct componentarray_wrapper final : icomponentarray {
    componentarray<T> _array;

    void destroy(entity e) noexcept override {
      if (_array.has(e)) {
        _array.remove(e);
      }
    }

    void clear() noexcept override {
      _array.clear();
    }
  };

  struct nodelete {
    void operator()(icomponentarray* ptr) const noexcept {
      if (ptr) {
        ptr->~icomponentarray();
      }
    }
  };

  using component_ptr = std::unique_ptr<icomponentarray, nodelete>;

public:
  componentmanager() noexcept {
    for (size_t i = 0; i < max_component_types; ++i) {
      _registered[i] = false;
    }
  }

  ~componentmanager() {
    for (auto& ptr : _component_arrays) {
      ptr.reset();
    }
  }

  componentmanager(const componentmanager&) = delete;
  componentmanager& operator=(const componentmanager&) = delete;
  componentmanager(componentmanager&&) = delete;
  componentmanager& operator=(componentmanager&&) = delete;

  template<typename T>
  void enroll() noexcept {
    auto const type = get_component_type<T>();

    assert(type < max_component_types && "Component type out of bounds.");
    assert(!_registered[type] && "Registering component type more than once.");
    assert(sizeof(componentarray_wrapper<T>) <= component_storage_size && "Component array too large.");

    auto* ptr = new (&_storage[type]) componentarray_wrapper<T>();
    _component_arrays[type] = component_ptr(ptr, nodelete{});
    _registered[type] = true;
  }

  template<typename T>
  [[nodiscard]] static constexpr componenttype get_component_type() noexcept {
    static const componenttype id = _next_component_type++;
    return id;
  }

  template<typename T>
  void add(entity e, T component) noexcept {
    _get_array<T>().insert(e, component);
  }

  template<typename T>
  void remove(entity e) noexcept {
    _get_array<T>().remove(e);
  }

  template<typename T>
  [[nodiscard]] T& get(entity e) noexcept {
    return _get_array<T>().get(e);
  }

  template<typename T>
  [[nodiscard]] const T& get(entity e) const noexcept {
    return _get_array<T>().get(e);
  }

  template<typename T>
  [[nodiscard]] bool has(entity e) const noexcept {
    auto const type = get_component_type<T>();
    return _registered[type] && _get_array<T>().has(e);
  }

  template<typename T>
  [[nodiscard]] componentarray<T>& get_array() noexcept {
    return _get_array<T>();
  }

  template<typename T>
  [[nodiscard]] const componentarray<T>& get_array() const noexcept {
    return _get_array<T>();
  }

  void destroy(entity e) noexcept {
    for (auto& ptr : _component_arrays) {
      if (ptr) {
        ptr->destroy(e);
      }
    }
  }

  void clear() noexcept {
    for (auto& ptr : _component_arrays) {
      if (ptr) {
        ptr->clear();
      }
    }
  }

private:
  template<typename T>
  [[nodiscard]] componentarray<T>& _get_array() noexcept {
    auto const type = get_component_type<T>();

    assert(type < max_component_types && "Component type out of bounds.");
    assert(_registered[type] && "Component not registered before use.");
    assert(_component_arrays[type] && "Component array is null.");

    auto* wrapper = static_cast<componentarray_wrapper<T>*>(_component_arrays[type].get());
    return wrapper->array;
  }

  template<typename T>
  [[nodiscard]] const componentarray<T>& _get_array() const noexcept {
    auto const type = get_component_type<T>();

    assert(type < max_component_types && "Component type out of bounds.");
    assert(_registered[type] && "Component not registered before use.");
    assert(_component_arrays[type] && "Component array is null.");

    auto* wrapper = static_cast<const componentarray_wrapper<T>*>(_component_arrays[type].get());
    return wrapper->array;
  }

  static inline componenttype _next_component_type{0};
  alignas(component_storage_align) std::byte _storage[max_component_types][component_storage_size];
  std::array<component_ptr, max_component_types> _component_arrays;
  bool _registered[max_component_types];
};
