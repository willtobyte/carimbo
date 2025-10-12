#include "overlay.hpp"

using namespace graphics;

overlay::overlay(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<input::eventmanager> eventmanager) noexcept
    : _resourcemanager(std::move(resourcemanager)), _eventmanager(std::move(eventmanager)) {}

std::variant<std::shared_ptr<label>> overlay::create(widgettype type) {
  const auto widget = [&]() {
    switch (type) {
    case widgettype::cursor:
      std::terminate();
    case widgettype::label:
      return std::make_shared<label>();
    }
  }();

  _widgets.emplace_back(widget);

  return widget;
}

void overlay::destroy(const std::variant<std::shared_ptr<label>>& widget) {
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

  if (auto cursor = _cursor; cursor) {
    cursor->update(delta);
  }
}

void overlay::draw() const noexcept {
  for (const auto& widget : _widgets) {
    widget->draw();
  }

  if (const auto cursor = _cursor; cursor) {
    cursor->draw();
  }
}

void overlay::set_cursor(const std::string& name) {
  _cursor = std::make_shared<cursor>(std::move(name), _resourcemanager);
  _eventmanager->add_receiver(_cursor);
}

void overlay::hide() {
  SDL_HideCursor();
}

void overlay::dispatch(widgettype type, const std::string& message) {
  UNUSED(message);

  switch (type) {
  case widgettype::cursor: {
    if (const auto cursor = _cursor; cursor) {
      cursor->handle(message);
    }
  } break;

  case widgettype::label: {
  } break;
  }
}
