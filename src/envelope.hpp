#pragma once

#include "common.hpp"

namespace framework {
struct collisionenvelope final {
  uint64_t a;
  uint64_t b;

  collisionenvelope(const uint64_t a, const uint64_t b) noexcept;
};

struct mailenvelope final {
  uint64_t to;
  std::string kind;
  std::string body;

  mailenvelope(const uint64_t to, const std::string_view kind, const std::string_view body);
};

struct timerenvelope final {
  bool repeat;
  std::function<void()> fn;

  timerenvelope(const bool repeat, std::function<void()>&& fn) noexcept;
};

using payload_t = std::variant<std::monostate, collisionenvelope, mailenvelope, timerenvelope>;

class envelope final {
public:
  payload_t payload;

  void reset(collisionenvelope&& envelope) noexcept;
  void reset(mailenvelope&& envelope);
  void reset(timerenvelope&& envelope) noexcept;
  void reset() noexcept;

  constexpr envelope() noexcept = default;
  constexpr ~envelope() = default;

  template<typename... Args>
    requires (!std::same_as<std::decay_t<Args>, envelope> && ...)
  constexpr explicit envelope(Args&&... args) {
    reset(std::forward<Args>(args)...);
  }

  [[nodiscard]] const collisionenvelope* try_collision() const noexcept;
  [[nodiscard]] const mailenvelope* try_mail() const noexcept;
  [[nodiscard]] const timerenvelope* try_timer() const noexcept;
};
}
