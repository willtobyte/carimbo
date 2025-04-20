#pragma once

#include "common.hpp"

namespace graphics {
enum class widgettype : uint8_t {
  cursor = 0,
  label,
};

class widget {
public:
  widget() = default;
  virtual ~widget() = default;

  virtual void update(float_t delta) = 0;

  virtual void draw() const = 0;
};
}
