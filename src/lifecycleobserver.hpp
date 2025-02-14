#pragma once

#include "common.hpp"

namespace framework {
class lifecycleobserver {
public:
  virtual ~lifecycleobserver() = default;

  virtual void on_beginupdate() noexcept;
  virtual void on_endupdate() noexcept;
  virtual void on_begindraw() noexcept;
  virtual void on_enddraw() noexcept;
};
}
