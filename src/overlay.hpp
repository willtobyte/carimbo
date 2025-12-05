#pragma once

#include "common.hpp"

#include "widget.hpp"

class overlay final : public eventreceiver {
public:
  overlay(std::shared_ptr<resourcemanager> resourcemanager, std::shared_ptr<eventmanager> eventmanager);
  virtual ~overlay() noexcept = default;

  std::variant<std::shared_ptr<label>> create(widgettype type) noexcept;

  void destroy(const std::variant<std::shared_ptr<label>>& widget) noexcept;

  void update(float delta) noexcept;

  void draw() const noexcept;

  void set_cursor(std::string_view name) noexcept;

  void hide(bool hidden = true) noexcept;

  void dispatch(widgettype type, std::string_view message) noexcept;

private:
  std::shared_ptr<resourcemanager> _resourcemanager;
  std::shared_ptr<eventmanager> _eventmanager;
  boost::container::small_vector<std::shared_ptr<widget>, 16> _widgets;
  std::shared_ptr<cursor> _cursor;
};