#include "eventreceiver.hpp"

void eventreceiver::on_quit() {}

void eventreceiver::on_key_press(const event::keyboard::key& event) {
}

void eventreceiver::on_key_release(const event::keyboard::key& event) {
}

void eventreceiver::on_text(std::string_view text) {
}

void eventreceiver::on_mouse_press(const event::mouse::button& event) {
}

void eventreceiver::on_mouse_release(const event::mouse::button& event) {
}

void eventreceiver::on_mouse_motion(const event::mouse::motion& event) {
}

#ifndef NDEBUG
void eventreceiver::on_debug() {
}
#endif
