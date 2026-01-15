#include "fontpool.hpp"

#include "font.hpp"

fontpool::fontpool(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {
  _fonts.reserve(8);
}

std::shared_ptr<font> fontpool::get(std::string_view family) {
  auto [it, inserted] = _fonts.try_emplace(family, nullptr);
  if (inserted) {
    it->second = std::make_shared<font>(_renderer, family);
  }

  return it->second;
}
