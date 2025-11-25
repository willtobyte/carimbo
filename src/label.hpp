#pragma once

#include "common.hpp"

#include "font.hpp"
#include "widget.hpp"


class label final : public widget {
public:
  label() = default;
  virtual ~label() = default;

  void set_font(std::shared_ptr<font> font) noexcept;

  void set(std::string_view text, float x, float y) noexcept;

  void set(float x, float y) noexcept;

  void set_effect(fonteffect::type type) noexcept;

  void clear() noexcept;

  virtual void update(float delta) noexcept override;

  virtual void draw() const noexcept override;

private:
  std::shared_ptr<font> _font;
  std::string _text;
  vec2 _position;
  std::shared_ptr<fonteffect> _effect;
};
