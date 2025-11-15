#include "locales.hpp"

static std::string_view language() {
  auto count = 0;
  std::unique_ptr<SDL_Locale*, SDL_Deleter> locales(SDL_GetPreferredLocales(&count));

  if (!locales || count == 0 || !locales.get()[0]) {
    return "en";
  }

  const SDL_Locale* locale = locales.get()[0];
  if (!locale || !locale->language) {
    return "en";
  }

  return locale->language;
}

static nlohmann::json parse(std::string_view code) {
  const auto filename = std::format("locales/{}.json", code);
  const auto& buffer = storage::io::read(filename);
  return nlohmann::json::parse(buffer);
}

static const nlohmann::json& mapping() {
  static const nlohmann::json j = [] {
    try {
      return parse(language());
    } catch (...) {
      return nlohmann::json{};
    }
  }();
  return j;
}

namespace localization {
std::string_view text(std::string_view key) {
  const auto& j = mapping();
  const auto it = j.find(key);

  if (it == j.end()) {
    return key;
  }

  return it.value().get<std::string_view>();
}
}
