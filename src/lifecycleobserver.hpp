#pragma once

#include "common.hpp"

class lifecycleobserver {
public:
  lifecycleobserver() = default;
  virtual ~lifecycleobserver() = default;

  virtual void on_beginupdate();
  virtual void on_endupdate();
  virtual void on_begindraw();
  virtual void on_enddraw();
};
