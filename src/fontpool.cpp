#include "fontpool.hpp"

#include "font.hpp"

fontpool::fontpool() {
  _fonts.reserve(8);
}

std::shared_ptr<font> fontpool::get(std::string_view family) {
  auto [it, inserted] = _fonts.try_emplace(family, nullptr);
  if (inserted) {
    it->second = std::make_shared<font>(family);
  }

  return it->second;
}
