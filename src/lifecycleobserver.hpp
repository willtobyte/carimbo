#pragma once

#include "common.hpp"

namespace framework {
class lifecycleobserver {
public:
  lifecycleobserver() noexcept = default;
  virtual ~lifecycleobserver() noexcept = default;

  virtual void on_beginupdate() noexcept;
  virtual void on_endupdate() noexcept;
  virtual void on_begindraw() noexcept;
  virtual void on_enddraw() noexcept;
};
}
