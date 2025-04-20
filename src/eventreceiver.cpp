#include "eventreceiver.hpp"

using namespace input;
using namespace event;

void eventreceiver::on_quit() {}

void eventreceiver::on_keydown(const keyboard::key &event) {
  UNUSED(event);
}

void eventreceiver::on_keyup(const keyboard::key &event) {
  UNUSED(event);
}

void eventreceiver::on_mousebuttondown(const mouse::button &event) {
  UNUSED(event);
}

void eventreceiver::on_mousebuttonup(const mouse::button &event) {
  UNUSED(event);
}

void eventreceiver::on_mousemotion(const mouse::motion &event) {
  UNUSED(event);
}

void eventreceiver::on_gamepadbuttondown(uint8_t who, const gamepad::button &event) {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_gamepadbuttonup(uint8_t who, const gamepad::button &event) {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_gamepadmotion(uint8_t who, const gamepad::motion &event) {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_mail(const mail &event) {
  UNUSED(event);
}

void eventreceiver::on_collision(const collision &event) {
  UNUSED(event);
}
