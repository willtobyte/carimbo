#include "eventreceiver.hpp"

using namespace input;
using namespace event;

void eventreceiver::on_quit() {}

void eventreceiver::on_key_press(const keyboard::key &event) {
  UNUSED(event);
}

void eventreceiver::on_key_release(const keyboard::key &event) {
  UNUSED(event);
}

void eventreceiver::on_mouse_press(const mouse::button &event) {
  UNUSED(event);
}

void eventreceiver::on_mouse_release(const mouse::button &event) {
  UNUSED(event);
}

void eventreceiver::on_mouse_motion(const mouse::motion &event) {
  UNUSED(event);
}

void eventreceiver::on_gamepad_press(uint8_t who, const gamepad::button &event) {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_gamepad_release(uint8_t who, const gamepad::button &event) {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_gamepad_motion(uint8_t who, const gamepad::motion &event) {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_mail(const mail &event) {
  UNUSED(event);
}

void eventreceiver::on_collision(const collision &event) {
  UNUSED(event);
}
