#pragma once

#include "common.hpp"
#include "widget.hpp"

namespace graphics {

class label : public widget {
public:
  label() = default;
  virtual ~label() = default;

  void set_font(std::shared_ptr<font> font) noexcept;
  void set_placement(const geometry::point &position) noexcept;

  void set(const std::string &text) noexcept;
  void set_with_placement(const std::string &text, int32_t x, int32_t y) noexcept;

  void update(float_t delta) noexcept override;
  void draw() const noexcept override;

private:
  std::shared_ptr<font> _font;
  std::string _text;
  geometry::point _position;
};
}
