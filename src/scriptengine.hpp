#pragma once

#include "common.hpp"

class scriptengine final : private noncopyable {
public:
  scriptengine() = default;
  virtual ~scriptengine() = default;

  void run();
};
