#pragma once

#include "common.hpp"

namespace framework {
class scriptengine : private noncopyable {
public:
  scriptengine() = default;
  virtual ~scriptengine() = default;

  void run();
};
}
