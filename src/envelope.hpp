#pragma once

#include "common.hpp"

struct mailenvelope final {
  uint64_t to;
  std::pmr::string kind;
  std::pmr::string body;

  explicit mailenvelope(std::pmr::memory_resource* mr = std::pmr::get_default_resource());
  mailenvelope(const uint64_t to, const std::string_view kind_view, const std::string_view body_view, std::pmr::memory_resource* mr = std::pmr::get_default_resource());

  void clear() noexcept;
};

struct timerenvelope final {
  bool repeat;
  std::function<void()> fn;

  timerenvelope(const bool repeat, std::function<void()>&& fn) noexcept;
  timerenvelope() noexcept;

  void clear() noexcept;
};

using payload_t = std::variant<std::monostate, mailenvelope, timerenvelope>;

class envelope final {
private:
  std::pmr::memory_resource* _mr;

public:
  payload_t payload;

  explicit envelope(std::pmr::memory_resource* mr = std::pmr::get_default_resource());
  ~envelope() = default;

  void reset(mailenvelope&& envelope);
  void reset(timerenvelope&& envelope) noexcept;
  void reset() noexcept;

  template<typename... Args>
  requires (!std::same_as<std::decay_t<Args>, envelope> && ...) &&
            (!std::is_pointer_v<std::remove_reference_t<Args>> && ...) &&
            (sizeof...(Args) > 0)
  explicit envelope(Args&&... args) : envelope() {
    reset(std::forward<Args>(args)...);
  }

  [[nodiscard]] const mailenvelope* try_mail() const noexcept;
  [[nodiscard]] const timerenvelope* try_timer() const noexcept;
};
