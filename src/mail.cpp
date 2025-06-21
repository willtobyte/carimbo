#include "mail.hpp"


using namespace framework;

mail::mail() noexcept
    : to(0) {}

mail::mail(
  std::shared_ptr<object> to,
  std::optional<std::shared_ptr<object>> from,
  const std::string& body
) noexcept
    : to(to->id()),
      kind(from && *from ? (*from)->kind() : "unknown"),
      body(body) {}

mail::mail(const mail& other) noexcept
    : to(other.to), kind(other.kind), body(other.body) {}

void mail::reset() noexcept {
  to = 0;
  kind.clear();
  body.clear();
}

void mail::reset(const mail& message) noexcept {
  to = message.to;
  kind = message.kind;
  body = message.body;
}
