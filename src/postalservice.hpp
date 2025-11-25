#pragma once

#include "common.hpp"

class envelope;

struct mail final {
  uint64_t to;
  std::string kind;
  std::string body;

  mail(
    std::shared_ptr<object> to,
    std::optional<std::shared_ptr<object>> from,
    std::string_view body
  ) noexcept
    : to(to->id()),
      kind(from && *from ? (*from)->kind() : "unknown"),
      body(body) {}
};

class postalservice final {
public:
  postalservice() noexcept;

  void post(const mail& message) noexcept;

private:
  std::shared_ptr<envelopepool_impl> _envelopepool;
};
