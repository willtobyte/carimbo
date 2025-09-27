#pragma once

#include "common.hpp"

#include "singleton.hpp"

#include "envelope.hpp"
#include "object.hpp"

namespace framework {

template<typename T, typename PtrType, auto& Name>
class poolbase {
protected:
  std::vector<PtrType> _objects;

  void expand(size_t minimum) {
    size_t target = _objects.capacity();
    if (target == 0) target = 1;

    while (target < minimum) target <<= 1;

    _objects.reserve(target);

    for (size_t i = _objects.size(); i < target; ++i) {
      if constexpr (std::is_same_v<PtrType, std::unique_ptr<T>>) _objects.emplace_back(std::make_unique<T>());
      if constexpr (std::is_same_v<PtrType, std::shared_ptr<T>>) _objects.emplace_back(std::make_shared<T>());
    }

    std::println("[pool<{}>] expanded to {} objects", Name, target);
  }

public:
  template<typename... Args>
  PtrType acquire(Args&&... args) {
    if (_objects.empty()) {
      const auto capacity = _objects.capacity();
      const auto need = capacity ? (capacity << 1) : 1;
      expand(need);
    }

    PtrType object = std::move(_objects.back());
    _objects.pop_back();

    if constexpr (requires { object->reset(std::forward<Args>(args)...); }) {
      object->reset(std::forward<Args>(args)...);
    }

    return object;
  }

  void release(PtrType&& object) {
    if (!object) {
      return;
    }

    if constexpr (std::is_same_v<PtrType, std::shared_ptr<T>>) {
      if (object.use_count() != MINIMAL_USE_COUNT) {
        return;
      }
    }

    _objects.emplace_back(std::move(object));
  }

  void reserve(size_t count) {
    expand(count);
  }

  size_t size() const {
    return _objects.size();
  }
};

template<typename T, auto& Name>
class uniquepool : public poolbase<T, std::unique_ptr<T>, Name> {
public:
  using poolbase<T, std::unique_ptr<T>, Name>::acquire;
  using poolbase<T, std::unique_ptr<T>, Name>::release;
};

inline constexpr char envelope_pool_name[] = "envelope";
using envelopepool = singleton<uniquepool<envelope, envelope_pool_name>>;

template<typename T, auto& Name>
class sharedpool : public poolbase<T, std::shared_ptr<T>, Name> {
public:
  using poolbase<T, std::shared_ptr<T>, Name>::acquire;
  using poolbase<T, std::shared_ptr<T>, Name>::release;
};

inline constexpr char object_pool_name[] = "object";
using objectpool = singleton<sharedpool<object, object_pool_name>>;
}
