#pragma once

#include "common.hpp"
#include "singleton.hpp"

namespace framework {

template<typename T>
class objectpool {
private:
  std::vector<std::unique_ptr<T>> objects;
  mutable std::mutex mutex;

  template<typename... Args>
  void expand(size_t min_new_objects, Args&&... args) {
    size_t new_capacity = std::max(min_new_objects, objects.capacity() ? objects.capacity() * 2 : 1);
    objects.reserve(new_capacity + objects.size());
    for (size_t i = 0; i < new_capacity; ++i) {
      objects.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }
  }

public:
  template<typename... Args>
  std::unique_ptr<T> acquire(Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex);

    if (objects.empty()) {
      expand(1, std::forward<Args>(args)...);
    }

    auto o = std::move(objects.back());
    objects.pop_back();

    if constexpr (requires { o->reset(std::forward<Args>(args)...); }) {
      o->reset(std::forward<Args>(args)...);
    }

    return o;
  }

  void release(std::unique_ptr<T> o) {
    if (!o) return;

    std::lock_guard<std::mutex> lock(mutex);
    objects.push_back(std::move(o));
  }

  template<typename... Args>
  void reserve(size_t count, Args&&... default_args) {
    std::lock_guard<std::mutex> lock(mutex);
    expand_pool(count, std::forward<Args>(default_args)...);
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return objects.size();
  }
};

using collision_pool = singleton<objectpool<collision>>;
using mail_pool = singleton<objectpool<mail>>;
using object_pool = singleton<objectpool<object>>;
}
