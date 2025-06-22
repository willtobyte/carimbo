#pragma once

#include "common.hpp"

#include "singleton.hpp"

#include "envelope.hpp"
#include "object.hpp"

namespace framework {

template<typename T, typename PtrType>
class poolbase {
protected:
  std::vector<PtrType> _objects;

  template<typename... Args>
  void expand(size_t minimum) {
    const auto target = std::max(minimum, _objects.empty() ? size_t(1) : _objects.size() * 2);

    _objects.reserve(target);

    for (size_t i = _objects.size(); i < target; ++i) {
      _objects.emplace_back(std::make_unique<T>());
    }
  }

public:
  template<typename... Args>
  PtrType acquire(Args&&... args) {
    if (_objects.empty()) {
      expand(size());
    }

    PtrType object = std::move(_objects.back());
    _objects.pop_back();

    if constexpr (requires { object->reset(std::forward<Args>(args)...); }) {
      object->reset(std::forward<Args>(args)...);
    }

    return object;
  }

  void release(PtrType object) {
    if (!object) {
      return;
    }

    if constexpr (std::is_same_v<PtrType, std::shared_ptr<T>>) {
      if (object.use_count() != MINIMAL_USE_COUNT) {
        return;
      }
    }

    _objects.push_back(std::move(object));
  }

  void reserve(size_t count) {
    expand(count);
  }

  size_t size() const {
    return _objects.size();
  }
};

template<typename T>
class sharedpool : public poolbase<T, std::shared_ptr<T>> {
public:
  using poolbase<T, std::shared_ptr<T>>::acquire;
  using poolbase<T, std::shared_ptr<T>>::release;
};

template<typename T>
class uniquepool : public poolbase<T, std::unique_ptr<T>> {
public:
  using poolbase<T, std::unique_ptr<T>>::acquire;
  using poolbase<T, std::unique_ptr<T>>::release;
};

using envelopepool = singleton<uniquepool<envelope>>;
using objectpool = singleton<sharedpool<object>>;
}
