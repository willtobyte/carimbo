#pragma once

#include "common.hpp"

#include "widget.hpp"

class overlay final : public eventreceiver {
public:
  explicit overlay(std::shared_ptr<resourcemanager> resourcemanager, std::shared_ptr<eventmanager> eventmanager);
  virtual ~overlay() = default;

  std::variant<std::shared_ptr<label>> create(widgettype type);

  void destroy(const std::variant<std::shared_ptr<label>>& widget);

  void update(float delta);

  void draw() const;

  void set_cursor(std::string_view name);

  void hide();

  void dispatch(widgettype type, std::string_view message);

private:
  std::shared_ptr<resourcemanager> _resourcemanager;
  std::shared_ptr<eventmanager> _eventmanager;
  std::vector<std::shared_ptr<widget>> _widgets;
  std::shared_ptr<cursor> _cursor;
};