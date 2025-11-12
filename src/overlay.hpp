#pragma once

#include "common.hpp"

#include "cursor.hpp"
#include "eventmanager.hpp"
#include "eventreceiver.hpp"
#include "label.hpp"
#include "resourcemanager.hpp"
#include "widget.hpp"

namespace graphics {
class overlay final : public input::eventreceiver {
public:
  explicit overlay(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<input::eventmanager> eventmanager);
  virtual ~overlay() = default;

  std::variant<std::shared_ptr<label>> create(widgettype type);

  void destroy(const std::variant<std::shared_ptr<label>>& widget);

  void update(float delta);

  void draw() const;

  void set_cursor(std::string_view name);

  void hide();

  void dispatch(widgettype type, std::string_view message);

private:
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<input::eventmanager> _eventmanager;
  std::vector<std::shared_ptr<graphics::widget>> _widgets;
  std::shared_ptr<graphics::cursor> _cursor;
};
}
