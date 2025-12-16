#pragma once

#include "common.hpp"

struct mailenvelope final {
  uint64_t to;
  std::pmr::string kind;
  std::pmr::string body;

  explicit mailenvelope(std::pmr::memory_resource* mr);
  void set(uint64_t to, std::string_view kind, std::string_view body);
  void clear() noexcept;
};

struct timerenvelope final {
  bool repeat;
  functor fn;

  void set(bool repeat, functor&& fn) noexcept;
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

  void reset(uint64_t to, std::string_view kind, std::string_view body);
  void reset(bool repeat, functor&& fn);
  void reset() noexcept;

  [[nodiscard]] const mailenvelope* try_mail() const noexcept;
  [[nodiscard]] const timerenvelope* try_timer() const noexcept;
};
