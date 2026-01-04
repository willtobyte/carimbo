#pragma once

#include "common.hpp"

#include "widget.hpp"

class overlay final : public eventreceiver {
public:
  overlay(std::shared_ptr<renderer> renderer, std::shared_ptr<eventmanager> eventmanager);
  virtual ~overlay() noexcept = default;

  std::variant<std::shared_ptr<label>, std::shared_ptr<cursor>> create(widgettype type, std::string_view resource);

  void destroy(const std::variant<std::shared_ptr<label>, std::shared_ptr<cursor>>& widget) noexcept;

  void update(float delta) noexcept;

  void draw() const noexcept;

  void dispatch(widgettype type, std::string_view message) noexcept;

private:
  std::shared_ptr<cursor> _cursor;
  std::shared_ptr<renderer> _renderer;
  std::shared_ptr<eventmanager> _eventmanager;
  boost::container::small_vector<std::shared_ptr<widget>, 16> _widgets;
  boost::unordered_flat_map<std::string, std::shared_ptr<font>, transparent_string_hash, std::equal_to<>> _fonts;
};
