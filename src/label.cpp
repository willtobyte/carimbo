#include "label.hpp"

#include "font.hpp"
#include "fonteffect.hpp"
#include "widget.hpp"

void label::set_font(std::shared_ptr<font> font) noexcept {
  _font = std::move(font);
}

void label::set(std::string_view text, float x, float y) noexcept {
  _text = text;
  _position = {x, y};
}

void label::set(float x, float y) noexcept {
  _position = {x, y};
}

void label::set_effect(fonteffect::type type) noexcept {
  switch (type) {
    case fonteffect::type::fadein:
      _effect = std::make_shared<fadeineffect>();
      break;
  }
}

void label::clear() noexcept {
  _text.clear();
  _position = {0, 0};
}

void label::update(float delta) noexcept {
  if (const auto e = _effect.get()) {
    e->update(delta);
  }
}

void label::draw() const noexcept {
  if (!_font || _text.empty()) [[unlikely]] {
    return;
  }

  if (const auto e = _effect.get()) {
    e->set(_text, _position);
  }

  _font->draw(_text, _position, _effect);
}
