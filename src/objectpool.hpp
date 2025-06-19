#pragma once

#include "common.hpp"
#include "singleton.hpp"

namespace framework {

template<typename T>
class objectpool {
private:
  std::vector<std::shared_ptr<T>> objects;

  template<typename... Args>
  void expand(size_t minimum, Args&&... args) {
    size_t ncapacity = std::max(minimum, objects.size() ? objects.size() * 2 : 1);
    for (size_t i = 0; i < ncapacity; ++i) {
      objects.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
    }
  }

public:
  template<typename... Args>
  std::shared_ptr<T> acquire(Args&&... args) {
    if (objects.empty()) {
      expand(size(), std::forward<Args>(args)...);
    }

    auto o = std::move(objects.back());
    objects.pop_back();

    if constexpr (requires { o->reset(std::forward<Args>(args)...); }) {
      o->reset(std::forward<Args>(args)...);
    }

    return o;
  }

  void release(std::shared_ptr<T> o) {
    if (o.use_count() == 1) {
      objects.push_back(std::move(o));
    }
  }

  template<typename... Args>
  void reserve(size_t count, Args&&... default_args) {
    expand(count, std::forward<Args>(default_args)...);
  }

  size_t size() const {
    return objects.size();
  }
};

using collision_pool = singleton<objectpool<collision>>;
using mail_pool = singleton<objectpool<mail>>;
using object_pool = singleton<objectpool<object>>;
}
