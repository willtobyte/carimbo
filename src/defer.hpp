#pragma once

template <typename F>
struct Defer {
  [[no_unique_address]] F f;

  explicit Defer(F&& fn) noexcept(std::is_nothrow_move_constructible_v<F>)
      : f(std::move(fn)) {}

  Defer(const Defer&) = delete;
  Defer& operator=(const Defer&) = delete;
  Defer(Defer&&) = delete;

  ~Defer() noexcept(noexcept(std::declval<F&>()())) {
    f();
  }
};

template <typename F>
[[nodiscard]] auto make_defer(F&& f) {
  return Defer<std::decay_t<F>>(std::forward<F>(f));
}

#define defer(code) auto _defer_##__LINE__ = make_defer([&](){ code; })
