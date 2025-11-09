#pragma once

#include "common.hpp"

namespace interop {
template<typename... Args>
static auto wrap_fn(sol::protected_function pf) -> std::function<void(Args...)> {
  return [pf = std::move(pf)](Args... args) mutable {
    auto result = pf(std::forward<Args>(args)...);
    if (!result.valid()) [[unlikely]] {
      sol::error error = result;
      throw std::runtime_error(error.what());
    }
  };
}

static auto wrap_fn(sol::protected_function pf) {
  return [pf = std::move(pf)](auto&&... args) mutable {
    auto result = pf(std::forward<decltype(args)>(args)...);
    if (!result.valid()) [[unlikely]] {
      sol::error error = result;
      throw std::runtime_error(error.what());
    }
  };
}
}

namespace framework {
class scriptengine final : private noncopyable {
public:
  scriptengine() = default;
  virtual ~scriptengine() = default;

  void run();
};
}
