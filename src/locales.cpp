#include "locales.hpp"

static std::string_view language() {
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

static nlohmann::json parse(std::string_view code) {
  const auto path = fmt::format("locales/{}.json", code);
  const auto data = storage::io::read(path);
  return nlohmann::json::parse(data.begin(), data.end());
}

static const nlohmann::json& mapping() {
  static const nlohmann::json j = [] {
    try {
      return parse(language());
    } catch (...) {
      return parse("en");
    }
  }();
  return j;
}

namespace framework {
std::string text(std::string_view key) {
  const auto& j = mapping();
  const auto it = j.find(std::string(key));

  if (it == j.end()) {
    return std::string(key);
  }

  return it.value().get<std::string>();
}
}
