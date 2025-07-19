#pragma once

namespace framework {
class loopable {
public:
  loopable() noexcept = default;
  virtual ~loopable() noexcept = default;

  virtual void loop(float_t delta) = 0;
};
}
