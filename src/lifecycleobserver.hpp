#pragma once

#include "common.hpp"

class lifecycleobserver {
public:
  lifecycleobserver() = default;
  virtual ~lifecycleobserver() = default;

  virtual void on_beginupdate() noexcept;
  virtual void on_endupdate() noexcept;
  virtual void on_begindraw() noexcept;
  virtual void on_enddraw() noexcept;
};
