#include "overlay.hpp"

#include "cursor.hpp"
#include "eventmanager.hpp"
#include "eventreceiver.hpp"
#include "label.hpp"
#include "resourcemanager.hpp"
#include "widget.hpp"

overlay::overlay(std::shared_ptr<resourcemanager> resourcemanager, std::shared_ptr<eventmanager> eventmanager)
    : _resourcemanager(std::move(resourcemanager)), _eventmanager(std::move(eventmanager)) {}

std::variant<std::shared_ptr<label>> overlay::create(widgettype type) noexcept {
  std::shared_ptr<label> widget;

  switch (type) {
  case widgettype::cursor:
    std::terminate();
  case widgettype::label:
    widget = std::make_shared<label>();
    break;
  default:
    widget = nullptr;
    break;
  }

  _widgets.emplace_back(widget);

  return widget;
}

void overlay::destroy(const std::variant<std::shared_ptr<label>>& widget) noexcept {
  std::erase_if(_widgets, [&widget](const auto& existing) {
    if (const auto ptr = std::get_if<std::shared_ptr<label>>(&widget)) {
      return existing == *ptr;
    }

    return false;
  });
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

void overlay::set_cursor(std::string_view name) noexcept {
  _cursor = std::make_shared<cursor>(name, _resourcemanager);
  _eventmanager->add_receiver(_cursor);
}

void overlay::hide(bool hidden) noexcept {
  hidden ? SDL_HideCursor() : SDL_ShowCursor();
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
