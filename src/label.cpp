#include "label.hpp"

using namespace graphics;

void label::set_font(std::shared_ptr<font> font) {
  _font = std::move(font);
}

void label::set_placement(const geometry::point &position) {
  _position = position;
}

void label::set(const std::string &text) {
  _text = text;
}

void label::set_with_placement(const std::string &text, float_t x, float_t y) {
  _text = text;
  _position = {x, y};
}

void label::clear() {
  _text.clear();
}

void label::update(float_t delta) {
  UNUSED(delta);
}

void label::draw() const {
  if (!_font || _text.empty()) [[unlikely]] {
    return;
  }

  _font->draw(_text, _position);
}
