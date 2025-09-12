#include "label.hpp"

using namespace graphics;

void label::set_font(std::shared_ptr<font> font) {
  _font = std::move(font);
}

void label::set(const std::string& text, float_t x, float_t y) {
  _text = text;
  _position = {x, y};
}

void label::set(float_t x, float_t y) {
  _position = {x, y};
}

void label::set_effect(fonteffect::type type) {
  switch (type) {
    case fonteffect::type::fadein:
      _effect = std::make_shared<fadeineffect>();
      break;
    default:
      break;
  }
}

void label::clear() {
  _text.clear();
  _position = {0, 0};
}

void label::update(float_t delta) noexcept {
  if (const auto e = _effect.get()) {
    e->update(delta);
  }
}

void label::draw() const  noexcept {
  if (!_font || _text.empty()) [[unlikely]] {
    return;
  }

  if (const auto e = _effect.get()) {
    e->set(_text, _position);
  }

  _font->draw(_text, _position, _effect);
}
