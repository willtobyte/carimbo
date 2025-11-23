#include "label.hpp"

#include "font.hpp"
#include "fonteffect.hpp"
#include "widget.hpp"

using namespace graphics;

void label::set_font(std::shared_ptr<font> font) {
  _font = std::move(font);
}

void label::set(std::string_view text, float x, float y) {
  _text = text;
  _position = {x, y};
}

void label::set(float x, float y) {
  _position = {x, y};
}

void label::set_effect(fonteffect::type type) {
  switch (type) {
    case fonteffect::type::fadein:
      _effect = std::make_shared<fadeineffect>();
      break;
  }
}

void label::clear() {
  _text.clear();
  _position = {0, 0};
}

void label::update(float delta) {
  if (const auto e = _effect.get()) {
    e->update(delta);
  }
}

void label::draw() const  {
  if (!_font || _text.empty()) [[unlikely]] {
    return;
  }

  if (const auto e = _effect.get()) {
    e->set(_text, _position);
  }

  _font->draw(_text, _position, _effect);
}
