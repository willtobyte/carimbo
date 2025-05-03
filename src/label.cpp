#include "label.hpp"

using namespace graphics;

void label::set_font(std::shared_ptr<font> font) {
  _font = std::move(font);
}

void label::set(const std::string &text, float_t x, float_t y) {
  _text = text;
  _position = {x, y};
}

void label::clear() {
  _text.clear();
  _position = {0, 0};
}

void label::update(float_t delta) {
  _font->update(delta);
}

void label::draw() const {
  if (!_font || _text.empty()) [[unlikely]] {
    return;
  }

  _font->draw(_text, _position);
}
