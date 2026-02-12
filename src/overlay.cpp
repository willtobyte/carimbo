#include "overlay.hpp"

#include "cursor.hpp"
#include "eventmanager.hpp"
#include "eventreceiver.hpp"
#include "fontpool.hpp"
#include "label.hpp"
#include "widget.hpp"

overlay::overlay(std::shared_ptr<eventmanager> eventmanager)
    : _eventmanager(std::move(eventmanager)) {}

void overlay::set_fontpool(std::shared_ptr<::fontpool> fontpool) noexcept {
  _fontpool = std::move(fontpool);
}


void overlay::cursor(std::string_view resource) {
  cursor(nullptr);

  _cursor = std::make_shared<::cursor>(resource);
  _eventmanager->add_receiver(_cursor);
}

void overlay::cursor(std::nullptr_t) {
  if (!_cursor) {
    return;
  }

  _eventmanager->remove_receiver(_cursor);
  _cursor.reset();
}

std::shared_ptr<::cursor> overlay::cursor() const noexcept {
  return _cursor;
}

std::shared_ptr<::label> overlay::label(std::string_view resource) {
  auto label = std::make_shared<::label>();
  label->set_font(_fontpool->get(resource));
  _labels.emplace_back(label);
  return label;
}

void overlay::label(std::shared_ptr<::label> instance) {
  _labels.erase(std::remove(_labels.begin(), _labels.end(), instance), _labels.end());
}

void overlay::dispatch(std::string_view message) noexcept {
  if (!_cursor) {
    return;
  }

  _cursor->handle(message);
}

void overlay::update(float delta) noexcept {
  for (const auto& label : _labels) {
    label->update(delta);
  }

  if (_cursor) {
    _cursor->update(delta);
  }
}

void overlay::draw() const {
  for (const auto& label : _labels) {
    label->draw();
  }

  if (_cursor) {
    _cursor->draw();
  }
}
