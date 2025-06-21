#pragma once

#include "common.hpp"

namespace framework {
struct mail final {
  uint64_t to{0};
  std::string kind;
  std::string body;

  mail(
    std::shared_ptr<object> to,
    std::optional<std::shared_ptr<object>> from,
    const std::string& body
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
  std::shared_ptr<uniquepool<envelope>> _envelopepool;
};
}
