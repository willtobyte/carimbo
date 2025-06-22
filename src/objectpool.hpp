#pragma once

#include "common.hpp"

#include "singleton.hpp"
#include "envelope.hpp"
#include "object.hpp"

namespace framework {
template<typename T, typename PtrType>
class poolbase {
protected:
  std::vector<PtrType> objects;

  template<typename... Args>
  void expand(size_t minimum) {
    const auto target = std::max(minimum, objects.empty() ? size_t(1) : objects.size() * 2);

    for (auto i = objects.size(); i < target; ++i) {
      objects.emplace_back(std::make_unique<T>());
    }
  }

public:
  template<typename... Args>
  PtrType acquire(Args&&... arguments) {
    if (objects.empty()) {
      expand(size());
    }

    PtrType object = std::move(objects.back());
    objects.pop_back();

    if constexpr (requires { object->reset(std::forward<Args>(arguments)...); }) {
      object->reset(std::forward<Args>(arguments)...);
    }

    return object;
  }

  void release(PtrType object) {
    if constexpr (std::is_same_v<PtrType, std::shared_ptr<T>>) {
      if (object.use_count() != MINIMAL_USE_COUNT) {
        return;
      }
    }

    objects.push_back(std::move(object));
  }

  template<typename... Args>
  void reserve(size_t count, Args&&... default_args) {
    expand(count, std::forward<Args>(default_args)...);
  }

  size_t size() const {
    return objects.size();
  }
};

template<typename T>
class sharedpool : public poolbase<T, std::shared_ptr<T>> {
public:
  using PtrType = std::shared_ptr<T>;

  template<typename... Args>
  PtrType acquire(Args&&... arguments) {
    return poolbase<T, PtrType>::acquire(std::forward<Args>(arguments)...);
  }

  void release(PtrType object) {
    poolbase<T, PtrType>::release(std::move(object));
  }
};

template<typename T>
class uniquepool : public poolbase<T, std::unique_ptr<T>> {
public:
  using PtrType = std::unique_ptr<T>;

  template<typename... Args>
  PtrType acquire(Args&&... arguments) {
    return poolbase<T, PtrType>::acquire(std::forward<Args>(arguments)...);
  }

  void release(PtrType object) {
    poolbase<T, PtrType>::release(std::move(object));
  }
};

using envelopepool = singleton<uniquepool<envelope>>;
using objectpool = singleton<sharedpool<object>>;
}
