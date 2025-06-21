#pragma once

#include "common.hpp"

#include "object.hpp"

namespace framework {
  struct mail final {
    uint64_t to;
    std::string kind;
    std::string body;

    mail() noexcept;

    mail(
      std::shared_ptr<object> to,
      std::optional<std::shared_ptr<object>> from,
      const std::string& body
    ) noexcept;

    mail(const mail& other) noexcept;

    void reset() noexcept;

    void reset(const mail& message) noexcept;
  };
}
