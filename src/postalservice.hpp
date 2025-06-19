#pragma once

#include "common.hpp"

namespace framework {
class postalservice final {
public:
  postalservice() noexcept;

  void post(const mail& message) noexcept;

private:
  std::shared_ptr<framework::objectpool<framework::mail>> _mail_pool;
};
}
