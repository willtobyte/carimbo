#pragma once

#include "common.hpp"

#include "singleton.hpp"

namespace framework {
template<typename T>
class objectpool {
private:
  std::vector<std::unique_ptr<T>> objects;
  mutable std::mutex mutex;

public:
  template<typename... Args>
  std::unique_ptr<T> acquire(Args&&... args) {
    fmt::println("acquire");
    std::lock_guard<std::mutex> lock(mutex);

    if (!objects.empty()) {
      auto o = std::move(objects.back());
      objects.pop_back();

      if constexpr (requires { o->reset(std::forward<Args>(args)...); }) {
        o->reset(std::forward<Args>(args)...);
      }

      return o;
    }

    return std::make_unique<T>(std::forward<Args>(args)...);
  }

  void release(std::unique_ptr<T> o) {
    fmt::println("release");
    if (o) {
      std::lock_guard<std::mutex> lock(mutex);
      objects.push_back(std::move(o));
    }
  }

  template<typename... Args>
  void reserve(size_t count, Args&&... default_args) {
    fmt::println("reserve {}", count);

    std::lock_guard<std::mutex> lock(mutex);
    objects.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      objects.push_back(std::make_unique<T>(std::forward<Args>(default_args)...));
    }
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return objects.size();
  }
};

using collision_pool = singleton<objectpool<collision>>;
using mail_pool = singleton<objectpool<mail>>;
}
