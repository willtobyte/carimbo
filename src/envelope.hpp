#pragma once

#include "common.hpp"

namespace framework {
struct collisionenvelope final {
  uint64_t a;
  uint64_t b;

  constexpr collisionenvelope(uint64_t a, uint64_t b) noexcept : a(a), b(b) {}
};

struct mailenvelope final {
  uint64_t to;
  std::string kind;
  std::string body;

  mailenvelope(uint64_t to_, const std::string& kind, const std::string& body) noexcept
    : to(to_), kind(kind), body(body) {}
};

struct timerenvelope final {
  bool repeat;
  std::function<void()> fn;

  timerenvelope(bool repeat_, std::function<void()>&& fn_) noexcept
    : repeat(repeat_), fn(std::move(fn_)) {}
};

using payload_t = std::variant<std::monostate, collisionenvelope, mailenvelope, timerenvelope>;

class envelope final {
public:
  payload_t payload;

  void reset(collisionenvelope&& envelope) noexcept;
  void reset(mailenvelope&& envelope) noexcept;
  void reset(timerenvelope&& envelope) noexcept;
  void reset() noexcept;

  envelope() noexcept = default;
  ~envelope() noexcept = default;

  template<typename... Args,
           typename = std::enable_if_t<!std::disjunction_v<std::is_same<std::decay_t<Args>, envelope>...>>>
  explicit envelope(Args&&... args) noexcept(noexcept(std::declval<envelope>().reset(std::forward<Args>(args)...))) {
    reset(std::forward<Args>(args)...);
  }

  [[nodiscard]] inline const auto& as_collision() const noexcept { return std::get<collisionenvelope>(payload); }
  [[nodiscard]] inline const auto& as_mail() const noexcept { return std::get<mailenvelope>(payload); }
  [[nodiscard]] inline const auto& as_timer() const noexcept { return std::get<timerenvelope>(payload); }
};
}
