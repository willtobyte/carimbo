#include "cursor.hpp"

using namespace graphics;

cursor::cursor(const std::string &name, std::shared_ptr<framework::resourcemanager> resourcemanager)
    : _x(0), _y(0), _resourcemanager(resourcemanager) {
  // SDL_ShowCursor(SDL_DISABLE);
  SDL_SetRelativeMouseMode(SDL_TRUE);

  UNUSED(name);

  _temp = _resourcemanager->pixmappool()->get("blobs/horn.png");
}

void cursor::on_mousemotion(const input::mousemotionevent &event) noexcept {
  const auto [x, y] = event;
  _x = x;
  _y = y;
}

void cursor::on_mousebuttondown(const input::mousebuttonevent &event) noexcept {
  UNUSED(event);
}

void cursor::on_mousebuttonup(const input::mousebuttonevent &event) noexcept {
  UNUSED(event);
}

void cursor::update(float_t delta) noexcept {
  UNUSED(delta);
}

void cursor::draw() const noexcept {
  _temp->draw(geometry::rect({0, 0}, {32, 32}), geometry::rect({_x, _y}, {32, 32}));
}
