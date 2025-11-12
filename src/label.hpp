#pragma once

#include "common.hpp"

#include "widget.hpp"

namespace graphics {

class label final : public widget {
public:
  label() = default;
  virtual ~label() = default;

  void set_font(std::shared_ptr<font> font);

  void set(std::string_view text, float x, float y);

  void set(float x, float y);

  void set_effect(fonteffect::type type);

  void clear();

  virtual void update(float delta) override;

  virtual void draw() const override;

private:
  std::shared_ptr<font> _font;
  std::string _text;
  geometry::point _position;
  std::shared_ptr<fonteffect> _effect;
};
}