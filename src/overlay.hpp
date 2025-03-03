#pragma once

#include "common.hpp"
#include "eventreceiver.hpp"
#include "widget.hpp"

namespace graphics {
class overlay : public input::eventreceiver {
public:
  explicit overlay(std::shared_ptr<renderer> renderer);
  ~overlay() = default;

  std::variant<std::shared_ptr<label>> create(widgettype type) noexcept;

  void destroy(std::variant<std::shared_ptr<label>> &&widget) noexcept;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  std::list<std::shared_ptr<widget>> _widgets;
};
}
