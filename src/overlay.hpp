#pragma once

#include "common.hpp"

#include "widget.hpp"

class overlay final : public eventreceiver {
public:
  explicit overlay(std::shared_ptr<eventmanager> eventmanager);
  virtual ~overlay() noexcept = default;

  void set_fontpool(std::shared_ptr<::fontpool> fontpool) noexcept;

  std::shared_ptr<::label> label(std::string_view font);
  void label(std::shared_ptr<::label> instance);

  void cursor(std::string_view resource);
  void cursor(std::nullptr_t);
  std::shared_ptr<::cursor> cursor() const noexcept;

  void dispatch(std::string_view message) noexcept;

  void update(float delta) noexcept;

  void draw() const;

private:
  std::shared_ptr<::cursor> _cursor;
  std::shared_ptr<fontpool> _fontpool;
  std::shared_ptr<eventmanager> _eventmanager;
  boost::container::small_vector<std::shared_ptr<widget>, 16> _labels;
};
