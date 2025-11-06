#pragma once

template <typename F>
struct _defer {
  [[no_unique_address]] F f;

  explicit _defer(F&& fn)
      : f(std::move(fn)) {}

  _defer(const _defer&) = delete;
  _defer& operator=(const _defer&) = delete;
  _defer(_defer&&) = delete;

  ~_defer() {
    f();
  }
};

template <typename F>
[[nodiscard]] auto make_defer(F&& f) {
  return _defer<std::decay_t<F>>(std::forward<F>(f));
}

#define defer(code) auto _defer_##__LINE__ = make_defer([&](){ code; })
