#pragma once

class loopable {
public:
  loopable() = default;
  virtual ~loopable() = default;

  virtual void loop(float delta) = 0;
};
