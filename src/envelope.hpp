#pragma once

#include "common.hpp"

namespace framework {
struct collisionenvelope final {
  uint64_t a;
  uint64_t b;

  constexpr collisionenvelope(const uint64_t a, const uint64_t b) noexcept : a(a), b(b) {}
};

struct mailenvelope final {
  uint64_t to;
  std::string kind;
  std::string body;

  constexpr mailenvelope(const uint64_t to, const std::string_view kind, const std::string_view body)
    : to(to), kind(kind), body(body) {}
};

struct timerenvelope final {
  bool repeat;
  std::function<void()> fn;

  constexpr timerenvelope(const bool repeat, std::function<void()>&& fn) noexcept
    : repeat(repeat), fn(std::move(fn)) {}
};

using payload_t = std::variant<std::monostate, collisionenvelope, mailenvelope, timerenvelope>;

class envelope final {
public:
  payload_t payload;

  constexpr void reset(collisionenvelope&& envelope) noexcept {
    payload.emplace<collisionenvelope>(std::move(envelope));
  }

  constexpr void reset(mailenvelope&& envelope) {
    payload.emplace<mailenvelope>(std::move(envelope));
  }

  constexpr void reset(timerenvelope&& envelope) noexcept {
    payload.emplace<timerenvelope>(std::move(envelope));
  }

  constexpr void reset() noexcept {
    payload.emplace<std::monostate>();
  }

  constexpr envelope() noexcept = default;
  constexpr ~envelope() = default;

  template<typename... Args>
    requires (!std::same_as<std::decay_t<Args>, envelope> && ...)
  constexpr explicit envelope(Args&&... args) {
    reset(std::forward<Args>(args)...);
  }

  [[nodiscard]] constexpr const collisionenvelope* try_collision() const noexcept {
    return std::get_if<collisionenvelope>(&payload);
  }

  [[nodiscard]] constexpr const mailenvelope* try_mail() const noexcept {
    return std::get_if<mailenvelope>(&payload);
  }

  [[nodiscard]] constexpr const timerenvelope* try_timer() const noexcept {
    return std::get_if<timerenvelope>(&payload);
  }
};
}