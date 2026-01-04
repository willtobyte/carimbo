#pragma once

#include "common.hpp"

class widget {
public:
  widget() = default;
  virtual ~widget() = default;

  virtual void update(float delta) = 0;

  virtual void draw() const = 0;
};
