#include "overlay.hpp"
#include "helpers.hpp"

using namespace graphics;

overlay::overlay(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {}

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
}

void overlay::draw() const noexcept {
  for (const auto &widget : _widgets) {
    widget->draw();
  }
}

void overlay::set_cursor(const std::string &name) noexcept {
  fmt::println("Set cursor {}", name);
  UNUSED(name);
}

void overlay::unset_cursor() noexcept {
}
