#include "label.hpp"

using namespace graphics;

void label::set_font(std::shared_ptr<font> font) noexcept {
  _font = std::move(font);
}

void label::set_placement(const geometry::point &position) noexcept {
  _position = position;
}

void label::set(const std::string &text) noexcept {
  _text = text;
}

void label::set_with_placement(const std::string &text, float_t x, float_t y) noexcept {
  _text = text;
  _position = {x, y};
}

void label::update(float_t delta) noexcept {
  UNUSED(delta);
}

void label::draw() const noexcept {
  if (!_font || _text.empty()) [[unlikely]] {
    return;
  }

  _font->draw(_text, _position);
}
