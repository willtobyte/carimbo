#pragma once

#include "common.hpp"

namespace interop {
template<typename Signature>
struct wrap_fn_impl;

template<typename... Args>
struct wrap_fn_impl<void(Args...)> {
  static auto wrap(const sol::protected_function& pf) -> std::function<void(Args...)> {
    return [pf](Args... args) {
      const auto result = pf(std::forward<Args>(args)...);
      if (!result.valid()) [[unlikely]] {
        const sol::error error = result;
        throw std::runtime_error(error.what());
      }
    };
  }
};

template<typename ReturnType, typename... Args>
struct wrap_fn_impl<ReturnType(Args...)> {
  static auto wrap(const sol::protected_function& pf) -> std::function<ReturnType(Args...)> {
    return [pf](Args... args) -> ReturnType {
      const auto result = pf(std::forward<Args>(args)...);
      if (!result.valid()) [[unlikely]] {
        const sol::error error = result;
        throw std::runtime_error(error.what());
      }

      return result.template get<ReturnType>();
    };
  }
};

template<typename Signature>
static auto wrap_fn(const sol::protected_function& pf) -> std::function<Signature> {
  return wrap_fn_impl<Signature>::wrap(pf);
}

static auto wrap_fn(const sol::protected_function& pf) {
  return [pf](auto&&... args) {
    const auto result = pf(std::forward<decltype(args)>(args)...);
    if (!result.valid()) [[unlikely]] {
      const sol::error error = result;
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