#include "overlay.hpp"
#include "helpers.hpp"

using namespace graphics;

overlay::overlay(const std::shared_ptr<framework::resourcemanager> resourcemanager, const std::shared_ptr<input::eventmanager> eventmanager)
    : _resourcemanager(resourcemanager), _eventmanager(eventmanager) {}

std::variant<std::shared_ptr<label>> overlay::create(widgettype type) noexcept {
  auto widget = [&]() noexcept {
    switch (type) {
    case widgettype::label:
      return std::make_shared<label>();
    }

    std::terminate();
  }();

  _widgets.emplace_back(widget);

  return widget;
}

void overlay::destroy(std::variant<std::shared_ptr<label>> &&widget) noexcept {
  std::visit(
      [this](auto &&argument) {
        std::erase_if(_widgets, [&argument](const auto &existing) {
          return existing == argument;
        });
      },
      widget
  );
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

void overlay::dispatch(const std::string &message) noexcept {
  if (const auto cursor = _cursor; cursor) {
    UNUSED(message);
    UNUSED(cursor);
    // cursor->handle(message);
  }
}
