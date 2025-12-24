#pragma once

#include "common.hpp"

#include "font.hpp"
#include "widget.hpp"

struct glypheffect {
  float xoffset{0.f};
  float yoffset{0.f};
  float scale{1.f};
  uint8_t r{255};
  uint8_t g{255};
  uint8_t b{255};
  uint8_t alpha{255};
};

class label final : public widget {
public:
  label() = default;
  virtual ~label() = default;

  void set_font(std::shared_ptr<font> font);

  void set(std::string_view text, float x, float y);

  void set(float x, float y);

  void set_effects(const boost::unordered_flat_map<size_t, std::optional<glypheffect>>& updates);

  void clear_effects() noexcept;

  void clear();

  virtual void update(float delta) override;

  virtual void draw() const override;

private:
  std::shared_ptr<font> _font;
  std::string _text;
  vec2 _position;
  boost::unordered_flat_map<size_t, glypheffect> _effects;
};
