#pragma once

#include "common.hpp"

#include "singleton.hpp"

namespace framework {

template<typename T, typename PtrType>
class poolbase {
protected:
  std::vector<PtrType> objects;

  template<typename... Args>
  void expand(size_t minimum, Args&&... args) {
    const auto target = std::max(minimum, objects.empty() ? size_t(1) : objects.size() * 2);
    for (size_t i = objects.size(); i < target; ++i) {
      objects.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    }
  }

public:
  template<typename... Args>
  PtrType acquire(Args&&... args) {
    if (objects.empty()) {
      expand(size(), std::forward<Args>(args)...);
    }

    PtrType obj = std::move(objects.back());
    objects.pop_back();

    if constexpr (requires { obj->reset(std::forward<Args>(args)...); }) {
      obj->reset(std::forward<Args>(args)...);
    }

    return obj;
  }

  void release(PtrType obj) {
    if constexpr (std::is_same_v<PtrType, std::shared_ptr<T>>) {
      if (obj.use_count() != MINIMAL_USE_COUNT) {
        return;
      }
    }
    objects.push_back(std::move(obj));
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
  PtrType acquire(Args&&... args) {
    return poolbase<T, PtrType>::acquire(std::forward<Args>(args)...);
  }

  void release(PtrType obj) {
    poolbase<T, PtrType>::release(std::move(obj));
  }
};

template<typename T>
class uniquepool : public poolbase<T, std::unique_ptr<T>> {
public:
  using PtrType = std::unique_ptr<T>;

  template<typename... Args>
  PtrType acquire(Args&&... args) {
    return poolbase<T, PtrType>::acquire(std::forward<Args>(args)...);
  }

  void release(PtrType obj) {
    poolbase<T, PtrType>::release(std::move(obj));
  }
};

using collisionpool = singleton<uniquepool<collision>>;
using mailpool = singleton<uniquepool<mail>>;
using objectpool = singleton<sharedpool<object>>;
}
