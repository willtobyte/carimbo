#pragma once

#include "common.hpp"

namespace framework {
struct mail final {
  uint64_t to;
  std::string kind;
  std::string body;

  mail(
    std::shared_ptr<object> to,
    std::optional<std::shared_ptr<object>> from,
    const std::string& body
  )
    : to(to->id()),
      kind(from && *from ? (*from)->kind() : "unknown"),
      body(body) {}
};

class postalservice final {
public:
  postalservice();

  void post(const mail& message);

private:
  std::shared_ptr<uniquepool<envelope, envelope_pool_name>> _envelopepool;
};
}
