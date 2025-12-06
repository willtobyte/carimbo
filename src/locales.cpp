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

[[nodiscard]] static const boost::unordered_flat_map<std::string, std::string, transparent_string_hash, std::equal_to<>>& mapping() noexcept {
  static struct {
    boost::unordered_flat_map<std::string, std::string, transparent_string_hash, std::equal_to<>> data;
    std::once_flag flag;

    decltype(data)& operator()() noexcept {
      std::call_once(flag, [this] {
        try {
          const auto filename = std::format("locales/{}.json", language());
          auto document = unmarshal::parse(io::read(filename));

          for (auto field : document.object()) {
            data.emplace(unmarshal::key(field), unmarshal::string(field.value()));
          }
        } catch (...) {
        }
      });

      return data;
    }
  } map;

  return map();
}

namespace localization {
[[nodiscard]] std::string_view text(std::string_view key) {
  const auto& m = mapping();

  if (const auto it = m.find(key); it != m.end()) [[likely]] {
    return it->second;
  }

  return key;
}
}
