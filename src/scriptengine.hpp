#pragma once

#include "common.hpp"
#include <concepts>
#include <type_traits>

namespace interop {

template<typename Signature>
struct wrap_fn_impl;

template<typename... Args>
struct wrap_fn_impl<void(Args...)> {
  static auto wrap(sol::protected_function pf) -> std::function<void(Args...)> {
    return [pf = std::move(pf)](Args... args) mutable {
      auto result = pf(std::forward<Args>(args)...);
      if (!result.valid()) [[unlikely]] {
        sol::error error = result;
        throw std::runtime_error(error.what());
      }
    };
  }
};

template<typename ReturnType, typename... Args>
struct wrap_fn_impl<ReturnType(Args...)> {
  static auto wrap(sol::protected_function pf) -> std::function<ReturnType(Args...)> {
    return [pf = std::move(pf)](Args... args) mutable -> ReturnType {
      auto result = pf(std::forward<Args>(args)...);
      if (!result.valid()) [[unlikely]] {
        sol::error error = result;
        throw std::runtime_error(error.what());
      }
      return result.template get<ReturnType>();
    };
  }
};

template<typename Signature>
auto wrap_fn(sol::protected_function pf) -> std::function<Signature> {
  return wrap_fn_impl<Signature>::wrap(std::move(pf));
}

auto wrap_fn(sol::protected_function pf) {
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
