#pragma once

#include "common.hpp"
#include "cursor.hpp"
#include "eventmanager.hpp"
#include "eventreceiver.hpp"
#include "resourcemanager.hpp"
#include "widget.hpp"

namespace graphics {
class overlay : public input::eventreceiver {
public:
  explicit overlay(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<input::eventmanager> eventmanager);
  ~overlay() = default;

  std::variant<std::shared_ptr<label>> create(widgettype type) noexcept;

  void destroy(const std::variant<std::shared_ptr<label>> &widget) noexcept;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

  void set_cursor(const std::string &name) noexcept;

  void dispatch(widgettype type, const std::string &message) noexcept;

private:
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<input::eventmanager> _eventmanager;
  std::vector<std::shared_ptr<widget>> _widgets;
  std::shared_ptr<cursor> _cursor;
};
}
