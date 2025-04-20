#pragma once

#include "common.hpp"

#include "cursor.hpp"
#include "eventmanager.hpp"
#include "eventreceiver.hpp"
#include "label.hpp"
#include "resourcemanager.hpp"
#include "widget.hpp"

namespace graphics {
class overlay : public input::eventreceiver {
public:
  explicit overlay(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<input::eventmanager> eventmanager);
  virtual ~overlay() = default;

  std::variant<std::shared_ptr<label>> create(widgettype type);

  void destroy(const std::variant<std::shared_ptr<label>> &widget);

  void update(float_t delta);

  void draw() const;

  void set_cursor(const std::string &name);

  void hide();

  void dispatch(widgettype type, const std::string &message);

private:
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<input::eventmanager> _eventmanager;
  std::vector<std::shared_ptr<widget>> _widgets;
  std::shared_ptr<cursor> _cursor;
};
}
