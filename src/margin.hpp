#pragma once

#include "common.hpp"

namespace geometry {
class margin final {
public:
  margin() = default;
  ~margin() = default;

  float top() const;
  void set_top(float value);

  float left() const;
  void set_left(float value);

  float bottom() const;
  void set_bottom(float value);

  float right() const;
  void set_right(float value);

  friend void from_json(const nlohmann::json& j, margin& m);

private:
  float _top{.0f};
  float _left{.0f};
  float _bottom{.0f};
  float _right{.0f};
};
}
