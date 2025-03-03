#include "eventreceiver.hpp"
#include <fmt/base.h>

using namespace input;

void eventreceiver::on_quit() noexcept {}

void eventreceiver::on_keydown(const keyevent &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_keyup(const keyevent &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_mousemotion(const mousemotionevent &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_mousebuttondown(const mousebuttonevent &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_mousebuttoup(const mousebuttonevent &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_joystickbuttondown(uint8_t who, const joystickevent &event) noexcept {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_joystickbuttonup(uint8_t who, const joystickevent &event) noexcept {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_joystickaxismotion(uint8_t who, const joystickaxisevent &event) noexcept {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_mail(const mailevent &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_collision(const collisionevent &event) noexcept {
  UNUSED(event);
}
