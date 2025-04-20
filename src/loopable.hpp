#pragma once

namespace framework {
class loopable {
public:
  loopable() = default;
  virtual ~loopable() = default;

  virtual void loop(float_t delta) = 0;
};
}
