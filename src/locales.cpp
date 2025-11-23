#include "locales.hpp"

static constexpr std::string_view DEFAULT_LANGUAGE = "en";

static std::string_view language() {
  auto count = 0;
  const auto locales = std::unique_ptr<SDL_Locale*, SDL_Deleter>(SDL_GetPreferredLocales(&count));

  if (!locales || count == 0) [[unlikely]] {
    return DEFAULT_LANGUAGE;
  }

  const auto* const locale = locales.get()[0];
  if (!locale || !locale->language) [[unlikely]] {
    return DEFAULT_LANGUAGE;
  }

  return locale->language;
}

[[nodiscard]] static nlohmann::json parse(std::string_view code) {
  const auto filename = std::format("locales/{}.json", code);
  const auto buffer = io::read(filename);
  return nlohmann::json::parse(buffer);
}

[[nodiscard]] static const nlohmann::json& mapping() noexcept {
  static const auto j = [] noexcept {
    try {
      return parse(language());
    } catch (...) {
      return nlohmann::json{};
    }
  }();
  return j;
}

namespace localization {
[[nodiscard]] std::string_view text(std::string_view key) {
  const auto j = mapping();

  if (const auto it = j.find(key); it != j.end()) [[likely]] {
    return it.value().get<std::string_view>();
  }

  return key;
}
}
