#pragma once

#include "common.hpp"

namespace framework {
struct collision_t {
  uint64_t a;
  uint64_t b;

  constexpr collision_t(uint64_t a, uint64_t b) noexcept : a(a), b(b) {}
};

struct mail_t {
  uint64_t to;
  std::string kind;
  std::string body;

  mail_t(uint64_t to_, const std::string& kind, const std::string& body) noexcept
    : to(to_), kind(kind), body(body) {}
};

struct timer_t {
  bool repeat;
  std::function<void()> fn;

  timer_t(bool repeat_, std::function<void()>&& fn_) noexcept
    : repeat(repeat_), fn(std::move(fn_)) {}
};

using payload_t = std::variant<std::monostate, collision_t, mail_t, timer_t>;

class envelope final {
public:
  payload_t payload;

  envelope() noexcept = default;
  ~envelope() noexcept = default;

  template<typename... Args,
           typename = std::enable_if_t<!std::disjunction_v<std::is_same<std::decay_t<Args>, envelope>...>>>
  explicit envelope(Args&&... args) noexcept(noexcept(std::declval<envelope>().reset(std::forward<Args>(args)...))) {
    reset(std::forward<Args>(args)...);
  }

  [[nodiscard]] inline auto& as_collision() noexcept { return std::get<collision_t>(payload); }
  [[nodiscard]] inline auto& as_mail() noexcept { return std::get<mail_t>(payload); }
  [[nodiscard]] inline auto& as_timer() noexcept { return std::get<timer_t>(payload); }

  void reset(uint64_t a, uint64_t b) noexcept;
  void reset(uint64_t to, const std::string& kind, const std::string& body) noexcept;
  void reset(bool repeat, std::function<void()>&& fn) noexcept;
  void reset() noexcept;
};
}
