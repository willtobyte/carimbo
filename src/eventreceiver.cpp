#include "eventreceiver.hpp"

using namespace input;

void eventreceiver::on_quit() noexcept {}

void eventreceiver::on_keydown(const keyevent &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_keyup(const keyevent &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_joystickbuttondown(int who, const joystickevent &event) noexcept {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_joystickbuttonup(int who, const joystickevent &event) noexcept {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_joystickaxismotion(int who, const joystickaxisevent &event) noexcept {
  UNUSED(who);
  UNUSED(event);
}

void eventreceiver::on_mail(const mailevent &event) noexcept {
  UNUSED(event);
}

void eventreceiver::on_collision(const collisionevent &event) noexcept {
  UNUSED(event);
}
