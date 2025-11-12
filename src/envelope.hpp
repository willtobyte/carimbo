#pragma once

#include "common.hpp"

namespace framework {
struct collisionenvelope final {
  uint64_t a;
  uint64_t b;

  constexpr collisionenvelope(uint64_t a, uint64_t b) : a(a), b(b) {}
};

struct mailenvelope final {
  uint64_t to;
  std::string kind;
  std::string body;

  mailenvelope(uint64_t to, std::string_view kind, std::string_view body)
    : to(to), kind(kind), body(body) {}
};

struct timerenvelope final {
  bool repeat;
  std::function<void()> fn;

  timerenvelope(bool repeat, std::function<void()>&& fn)
    : repeat(repeat), fn(std::move(fn)) {}
};

using payload_t = std::variant<std::monostate, collisionenvelope, mailenvelope, timerenvelope>;

class envelope final {
public:
  payload_t payload;

  void reset(collisionenvelope&& envelope);
  void reset(mailenvelope&& envelope);
  void reset(timerenvelope&& envelope);
  void reset();

  envelope() = default;
  ~envelope() = default;

  template<typename... Args,
           typename = std::enable_if_t<!std::disjunction_v<std::is_same<std::decay_t<Args>, envelope>...>>>
  explicit envelope(Args&&... args) {
    reset(std::forward<Args>(args)...);
  }

  [[nodiscard]] inline const collisionenvelope* try_collision() const {
    return std::get_if<collisionenvelope>(&payload);
  }

  [[nodiscard]] inline const mailenvelope* try_mail() const {
    return std::get_if<mailenvelope>(&payload);
  }

  [[nodiscard]] inline const timerenvelope* try_timer() const {
    return std::get_if<timerenvelope>(&payload);
  }
};
}
