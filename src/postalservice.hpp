#pragma once

#include "common.hpp"

namespace framework {
struct mail final {
  uint64_t to;
  std::string kind;
  std::string body;

  mail(std::shared_ptr<object> to, std::optional<std::shared_ptr<object>> from, const std::string &body)
    : to(to->id()),
      kind(from && *from ? (*from)->kind() : "unknown"),
      body(body) {}

  mail(const mail &other)
    : to(other.to), kind(other.kind), body(other.body) {}
};

class postalservice final {
public:
  postalservice() = default;

  void post(const mail &message);
};
}
