#pragma once

#include "common.hpp"

namespace framework {

enum class envelopekind : uint8_t {
  none,
  collision,
  mail,
  timer,
};

class envelope final {
public:
  envelopekind kind = envelopekind::none;

  union content {
    struct {
      uint64_t a;
      uint64_t b;
    } collision;

    struct {
      uint64_t to;
      std::string kind;
      std::string body;
    } mail;

    struct {
      bool repeat;
      std::function<void()> fn;
    } timer;

    content() noexcept {}
    ~content() noexcept {}
  } payload;

  envelope() noexcept = default;

  template<typename... Args>
  explicit envelope(Args&&... args) noexcept {
    reset(std::forward<Args>(args)...);
  }

  ~envelope() noexcept {
    clear();
  }

  auto& as_collision() noexcept { return payload.collision; }
  auto& as_timer() noexcept { return payload.timer; }
  auto& as_mail() noexcept { return payload.mail; }

  void reset(uint64_t a, uint64_t b) noexcept {
    clear();
    kind = envelopekind::collision;
    payload.collision.a = a;
    payload.collision.b = b;
  }

  void reset(uint64_t to, std::string&& kind_str, std::string&& body_str) noexcept {
    clear();
    kind = envelopekind::mail;
    new (&payload.mail.kind) std::string(std::move(kind_str));
    new (&payload.mail.body) std::string(std::move(body_str));
    payload.mail.to = to;
  }

  void reset(bool repeat, std::function<void()>&& fn) noexcept {
    clear();
    kind = envelopekind::timer;
    payload.timer.repeat = repeat;
    new (&payload.timer.fn) std::function<void()>(std::move(fn));
  }

private:
  void clear() noexcept {
    if (kind == envelopekind::mail) {
      payload.mail.kind.~basic_string();
      payload.mail.body.~basic_string();
      kind = envelopekind::none;
      return;
    }

    if (kind == envelopekind::timer) {
      payload.timer.fn.~function();
      kind = envelopekind::none;
      return;
    }

    kind = envelopekind::none;
  }
};
}
