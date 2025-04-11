#include "eventreceiver.hpp"

using namespace input;
using namespace event;

void eventreceiver::on_quit() noexcept {}

void eventreceiver::on_keydown(const keyboard::key &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_keyup(const keyboard::key &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_mousebuttondown(const mouse::button &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_mousebuttonup(const mouse::button &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_mousemotion(const mouse::motion &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_gamepadbuttondown(uint8_t who, const gamepad::button &event) noexcept {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_gamepadbuttonup(uint8_t who, const gamepad::button &event) noexcept {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_gamepadmotion(uint8_t who, const gamepad::motion &event) noexcept {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_mail(const mail &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_collision(const collision &event) noexcept {
  UNUSED(event);
}
