#include "overlay.hpp"

#include "cursor.hpp"
#include "eventmanager.hpp"
#include "eventreceiver.hpp"
#include "font.hpp"
#include "label.hpp"
#include "renderer.hpp"
#include "widget.hpp"

overlay::overlay(std::shared_ptr<renderer> renderer, std::shared_ptr<eventmanager> eventmanager)
    : _renderer(std::move(renderer)), _eventmanager(std::move(eventmanager)) {}

std::shared_ptr<::font> overlay::preload(std::string_view resource) {
  auto [it, inserted] = _fonts.try_emplace(resource, nullptr);
  if (inserted) {
    it->second = std::make_shared<::font>(_renderer, resource);
  }

  return it->second;
}

std::shared_ptr<::label> overlay::label(std::string_view resource) {
  auto label = std::make_shared<::label>();
  label->set_font(preload(resource));
  _labels.emplace_back(label);
  return label;
}

void overlay::label(std::shared_ptr<::label> instance) {
  _labels.erase(std::remove(_labels.begin(), _labels.end(), instance), _labels.end());
}

void overlay::cursor(std::string_view resource) {
  cursor(nullptr);
  _cursor = std::make_shared<::cursor>(resource, _renderer);
  _eventmanager->add_receiver(_cursor);
}

void overlay::cursor(std::nullptr_t) {
  if (!_cursor) {
    return;
  }

  _eventmanager->remove_receiver(_cursor);
  _cursor.reset();
}

void overlay::dispatch(std::string_view message) noexcept {
  if (!_cursor) {
    return;
  }

  _cursor->handle(message);
}

void overlay::update(float delta) noexcept {
  for (const auto& label : _labels) {
    label->update(delta);
  }

  if (_cursor) {
    _cursor->update(delta);
  }
}

void overlay::draw() const {
  for (const auto& label : _labels) {
    label->draw();
  }

  if (_cursor) {
    _cursor->draw();
  }
}
