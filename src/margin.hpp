#pragma once

#include "common.hpp"

namespace geometry {
class margin final {
public:
  margin() noexcept = default;
  ~margin() noexcept = default;

  int32_t top() const noexcept;
  void set_top(int32_t value) noexcept;

  int32_t left() const noexcept;
  void set_left(int32_t value) noexcept;

  int32_t bottom() const noexcept;
  void set_bottom(int32_t value) noexcept;

  int32_t right() const noexcept;
  void set_right(int32_t value) noexcept;

  friend void from_json(const nlohmann::json &j, margin &m);

private:
  int32_t _top{0};
  int32_t _left{0};
  int32_t _bottom{0};
  int32_t _right{0};
};
}
