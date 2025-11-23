#pragma once

#include "common.hpp"

enum class widgettype : uint8_t {
  cursor = 0,
  label,
};

class widget {
public:
  widget() = default;
  virtual ~widget() = default;

  virtual void update(float delta) = 0;

  virtual void draw() const = 0;
};
