#include "locales.hpp"

static std::string language() {
  auto count = 0;
  const auto locales = SDL_GetPreferredLocales(&count);
  if (!locales || count == 0) {
    return "en";
  }

  const SDL_Locale* locale = locales[0];
  if (!locale || !locale->language) {
    return "en";
  }

  return locale->language;
}

static nlohmann::json parse(const std::string& code) {
  const auto& filename = std::format("locales/{}.json", code);
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

namespace framework {
std::string text(const std::string& key) {
  const auto& j = mapping();
  const auto it = j.find(key);

  if (it == j.end()) {
    return key;
  }

  return it.value().get<std::string>();
}
}
