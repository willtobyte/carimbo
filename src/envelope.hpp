#pragma once

#include "common.hpp"

namespace framework {
enum class envelopekind : uint8_t {
  none,
  collision,
  mail,
  timer,
};

struct collision_t {
  uint64_t a;
  uint64_t b;
};

struct mail_t {
  uint64_t to;
  std::string kind;
  std::string body;
};

struct timer_t {
  bool repeat;
  std::function<void()> fn;
};

using payload_t = std::variant<std::monostate, collision_t, mail_t, timer_t>;

class envelope final {
public:
  envelopekind kind = envelopekind::none;
  payload_t payload;

  envelope() noexcept = default;

  template<typename... Args>
  explicit envelope(Args&&... args) {
    reset(std::forward<Args>(args)...);
  }

  auto& as_collision() noexcept { return std::get<collision_t>(payload); }
  auto& as_mail() noexcept { return std::get<mail_t>(payload); }
  auto& as_timer() noexcept { return std::get<timer_t>(payload); }

  void reset(uint64_t a, uint64_t b) noexcept;
  void reset(uint64_t to, std::string&& kind_str, std::string&& body_str) noexcept;
  void reset(bool repeat, std::function<void()>&& fn) noexcept;
  void reset() noexcept;
};
}
