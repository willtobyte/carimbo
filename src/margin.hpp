#pragma once

#include "common.hpp"

namespace geometry {
class margin final {
public:
  margin() noexcept = default;
  ~margin() noexcept = default;

  float top() const noexcept;
  void set_top(float value) noexcept;

  float left() const noexcept;
  void set_left(float value) noexcept;

  float bottom() const noexcept;
  void set_bottom(float value) noexcept;

  float right() const noexcept;
  void set_right(float value) noexcept;

  friend void from_json(const nlohmann::json& j, margin& m);

private:
  float _top{.0f};
  float _left{.0f};
  float _bottom{.0f};
  float _right{.0f};
};
}
