#pragma once

#include "common.hpp"
#include "widget.hpp"

namespace graphics {

class label : public widget {
public:
  label() = default;
  virtual ~label() = default;

  void set_font(std::shared_ptr<font> font);
  void set_placement(const geometry::point &position);

  void set(const std::string &text);
  void set_with_placement(const std::string &text, float_t x, float_t y);

  void clear();

  void update(float_t delta) override;
  void draw() const override;

private:
  std::shared_ptr<font> _font;
  std::string _text;
  geometry::point _position;
};
}
