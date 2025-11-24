#pragma once

#include "common.hpp"
#include "entity.hpp"
#include "system.hpp"

class systemmanager final {
  static constexpr auto max_systems = 32uz;
  static constexpr auto max_system_size = 1024uz;
  static constexpr auto system_alignment = 64uz;

  struct nodelete {
    void operator()(class system* ptr) const noexcept {
      if (ptr) {
        ptr->~system();
      }
    }
  };

  using system_ptr = std::unique_ptr<class system, nodelete>;

public:
  systemmanager() noexcept : _count(0) {}

  ~systemmanager() {
    for (auto& ptr : _systems) {
      ptr.reset();
    }
  }

  systemmanager(const systemmanager&) = delete;
  systemmanager& operator=(const systemmanager&) = delete;
  systemmanager(systemmanager&&) = delete;
  systemmanager& operator=(systemmanager&&) = delete;

  template<typename T, typename... Args>
  T& register_system(Args&&... args) noexcept {
    static_assert(std::is_base_of_v<class system, T>, "T must derive from system");
    static_assert(sizeof(T) <= max_system_size, "System too large");
    static_assert(alignof(T) <= system_alignment, "System alignment too large");
    
    assert(_count < max_systems && "Maximum systems reached.");

    _systems[_count] = system_ptr(
      new (&_storage[_count]) T(std::forward<Args>(args)...),
      nodelete{}
    );
    auto& result = static_cast<T&>(*_systems[_count]);
    ++_count;
    
    return result;
  }

  template<typename T>
  void set_signature(signature sig) noexcept {
    auto sys = _find_system<T>();
    assert(sys && "System not registered.");
    sys->set_signature(sig);
  }

  void entity_destroyed(entity e) noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (_systems[i] && _systems[i]->has_entity(e)) {
        _systems[i]->remove_entity(e);
      }
    }
  }

  void entity_signature_changed(entity e, signature sig) noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (!_systems[i]) continue;
      
      auto& sys = *_systems[i];
      auto const& sys_sig = sys.get_signature();
      auto const matches = signature_ops::matches(sig, sys_sig);
      auto const has = sys.has_entity(e);

      if (matches && !has) {
        sys.add_entity(e);
      } else if (!matches && has) {
        sys.remove_entity(e);
      }
    }
  }

  void update(float delta) noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (_systems[i]) {
        _systems[i]->update(delta);
      }
    }
  }

  void clear() noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (_systems[i]) {
        _systems[i]->clear_entities();
      }
    }
  }

  [[nodiscard]] size_t count() const noexcept {
    return _count;
  }

  [[nodiscard]] bool empty() const noexcept {
    return _count == 0;
  }

  template<typename T>
  [[nodiscard]] std::shared_ptr<T> get_system() noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (_systems[i]) {
        if (auto& sys = *_systems[i]; typeid(sys) == typeid(T)) {
          return std::shared_ptr<T>(
            static_cast<T*>(&sys),
            [](T*) {}
          );
        }
      }
    }
    return nullptr;
  }

  template<typename T>
  [[nodiscard]] std::shared_ptr<const T> get_system() const noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (_systems[i]) {
        if (auto& sys = *_systems[i]; typeid(sys) == typeid(T)) {
          return std::shared_ptr<const T>(
            static_cast<const T*>(&sys),
            [](const T*) {}
          );
        }
      }
    }
    return nullptr;
  }

private:
  template<typename T>
  [[nodiscard]] T* _find_system() noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (_systems[i]) {
        if (auto* ptr = dynamic_cast<T*>(_systems[i].get())) {
          return ptr;
        }
      }
    }
    return nullptr;
  }

  template<typename T>
  [[nodiscard]] const T* _find_system() const noexcept {
    for (size_t i = 0; i < _count; ++i) {
      if (_systems[i]) {
        if (auto* ptr = dynamic_cast<const T*>(_systems[i].get())) {
          return ptr;
        }
      }
    }
    return nullptr;
  }

  alignas(system_alignment) std::byte _storage[max_systems][max_system_size];
  std::array<system_ptr, max_systems> _systems;
  size_t _count;
};