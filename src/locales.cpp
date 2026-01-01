#include "locales.hpp"

static constexpr std::string DEFAULT_LANGUAGE = "en";

static std::string language() {
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

    decltype(data)& operator*() noexcept {
      try {
        const auto filename = std::format("locales/{}.json", language());
        auto json = unmarshal::parse(io::read(filename));

        size_t index, max;
        yyjson_val *k, *v;
        yyjson_obj_foreach(*json, index, max, k, v) {
          data.emplace(unmarshal::key(k), unmarshal::string(v));
        }
      } catch (...) {
      }

      return data;
    }
  } map;

  return *map;
}

namespace localization {
[[nodiscard]] std::string_view text(std::string_view key) {
  static const auto& m = mapping();

  if (const auto it = m.find(key); it != m.end()) [[likely]] {
    return it->second;
  }

  return key;
}
}
