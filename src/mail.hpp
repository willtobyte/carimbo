#pragma once

#include "common.hpp"

#include "object.hpp"

namespace framework {
  struct mail final {
    uint64_t to{};
    std::string kind;
    std::string body;

    mail() noexcept = default;

    mail(
      std::shared_ptr<object> to,
      std::optional<std::shared_ptr<object>> from,
      const std::string& body
    ) noexcept
      : to(to->id()),
        kind(from && *from ? (*from)->kind() : "unknown"),
        body(body) {}

    mail(const mail& other) noexcept
      : to(other.to), kind(other.kind), body(other.body) {}

    constexpr void reset() noexcept {
      to = 0;
      kind.clear();
      body.clear();
    }

    constexpr void reset(uint64_t to, const std::string& kind, const std::string& body) noexcept {
      this->to = to;
      this->kind = kind;
      this->body = body;
    }
  };
}
