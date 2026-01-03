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

std::variant<std::shared_ptr<label>, std::shared_ptr<cursor>> overlay::create(widgettype type, std::string_view resource) {
  switch (type) {
  case widgettype::cursor: {
    _cursor = std::make_shared<cursor>(resource, _renderer);
    _eventmanager->add_receiver(_cursor);
    return _cursor;
  }
  case widgettype::label: {
    auto widget = std::make_shared<label>();
    auto it = _fonts.find(resource);
    if (it == _fonts.end()) {
      it = _fonts.emplace(std::string{resource}, std::make_shared<font>(_renderer, resource)).first;
    }
    widget->set_font(it->second);
    _widgets.emplace_back(widget);
    return widget;
  }
  }

  std::unreachable();
}

void overlay::destroy(const std::variant<std::shared_ptr<label>, std::shared_ptr<cursor>>& widget) noexcept {
  if (const auto ptr = std::get_if<std::shared_ptr<cursor>>(&widget)) {
    if (_cursor == *ptr) {
      _eventmanager->remove_receiver(_cursor);
      _cursor.reset();
    }

    return;
  }

  if (const auto ptr = std::get_if<std::shared_ptr<label>>(&widget)) {
    _widgets.erase(std::remove(_widgets.begin(), _widgets.end(), *ptr), _widgets.end());
  }
}

void overlay::update(float delta) noexcept {
  for (const auto& widget : _widgets) {
    widget->update(delta);
  }

  if (const auto& cursor = _cursor; cursor) {
    cursor->update(delta);
  }
}

void overlay::draw() const noexcept {
  for (const auto& widget : _widgets) {
    widget->draw();
  }

  if (const auto& cursor = _cursor; cursor) {
    cursor->draw();
  }
}

void overlay::dispatch(widgettype type, std::string_view message) noexcept {
  switch (type) {
  case widgettype::cursor: {
    if (const auto& cursor = _cursor; cursor) {
      cursor->handle(message);
    }
  } break;

  case widgettype::label: {
  } break;
  }
}
