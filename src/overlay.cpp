#include "overlay.hpp"
#include "helpers.hpp"
#include "widget.hpp"

using namespace graphics;

overlay::overlay(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<input::eventmanager> eventmanager)
    : _resourcemanager(resourcemanager), _eventmanager(eventmanager) {}

std::variant<std::shared_ptr<label>> overlay::create(widgettype type) noexcept {
  auto widget = [&]() noexcept {
    switch (type) {
    case widgettype::label:
      return std::make_shared<label>();
    default:
      std::terminate();
    }
  }();

  _widgets.emplace_back(widget);

  return widget;
}

void overlay::destroy(const std::variant<std::shared_ptr<label>> &widget) noexcept {
  std::erase_if(_widgets, [&widget](const auto &existing) {
    if (const auto ptr = std::get_if<std::shared_ptr<label>>(&widget)) {
      return existing == *ptr;
    }

    return false;
  });
}

void overlay::update(float_t delta) noexcept {
  for (const auto &widget : _widgets) {
    widget->update(delta);
  }

  if (auto cursor = _cursor; cursor) {
    cursor->update(delta);
  }
}

void overlay::draw() const noexcept {
  for (const auto &widget : _widgets) {
    widget->draw();
  }

  if (const auto cursor = _cursor; cursor) {
    cursor->draw();
  }
}

void overlay::set_cursor(const std::string &name) noexcept {
  _cursor = std::make_shared<cursor>(name, _resourcemanager);
  _eventmanager->add_receiver(_cursor);
}

void overlay::dispatch(widgettype type, const std::string &message) noexcept {
  UNUSED(message);

  switch (type) {
  case widgettype::cursor: {
    if (const auto cursor = _cursor; cursor) {
      cursor->handle(message);
    }

  } break;

  default:
    break;
  }
}
