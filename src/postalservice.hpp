#pragma once

#include "common.hpp"

namespace framework {
struct mail {
  uint64_t to;
  std::string kind;
  std::string body;

  mail(std::shared_ptr<entity> to, std::shared_ptr<entity> from, const std::string &body)
      : to(to->id()), kind(from->kind()), body(body) {}
};

class postalservice {
public:
  postalservice() noexcept = default;

  void post(const mail &message) noexcept;
};
}
