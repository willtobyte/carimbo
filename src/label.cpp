#include "label.hpp"

#include "font.hpp"

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

void label::set_effects(const boost::unordered_flat_map<size_t, std::optional<glypheffect>>& updates) {
  for (const auto& [index, props] : updates) {
    if (props) {
      _effects[index] = *props;
    } else {
      _effects.erase(index);
    }
  }
}

void label::clear_effects() noexcept {
  _effects.clear();
}

void label::clear() {
  _text.clear();
  _position = {0, 0};
  _effects.clear();
}

std::string_view label::glyphs() const noexcept {
  return _font->glyphs();
}

void label::update(float delta) {
}

void label::draw() const {
  if (!_font || _text.empty()) [[unlikely]] {
    return;
  }

  _font->draw(_text, _position, _effects);
}
