#pragma once

#include "common.hpp"

namespace interop {

template<typename T>
  requires requires(const T& t) { { t.valid() } -> std::convertible_to<bool>; }
inline void verify(const T& result,
                   std::source_location loc = std::source_location::current()) noexcept(false) {
  if (!result.valid()) [[unlikely]] {
    sol::error err = result;
    throw std::runtime_error(std::format("{}:{} - {}", loc.file_name(), loc.line(), err.what()));
  }
}

template<typename Signature>
struct wrap_fn_impl;

template<typename... Args>
struct wrap_fn_impl<void(Args...)> {
  static auto wrap(sol::protected_function pf) -> std::function<void(Args...)> {
    return [pf = std::move(pf)](Args... args) mutable {
      auto result = pf(std::forward<Args>(args)...);
      verify(result);
    };
  }
};

template<typename ReturnType, typename... Args>
struct wrap_fn_impl<ReturnType(Args...)> {
  static auto wrap(sol::protected_function pf) -> std::function<ReturnType(Args...)> {
    return [pf = std::move(pf)](Args... args) mutable -> ReturnType {
      auto result = pf(std::forward<Args>(args)...);
      verify(result);
      return result.template get<ReturnType>();
    };
  }
};

template<typename Signature>
static auto wrap_fn(sol::protected_function pf) -> std::function<Signature> {
  return wrap_fn_impl<Signature>::wrap(std::move(pf));
}

static auto wrap_fn(sol::protected_function pf) {
  return [pf = std::move(pf)](auto&&... args) mutable {
    auto result = pf(std::forward<decltype(args)>(args)...);
    verify(result);
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
