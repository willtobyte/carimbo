#include "cassette.hpp"

using namespace storage;

cassette::cassette() {
#ifdef EMSCRIPTEN
  const auto* const raw = emscripten_run_script_string("document.cookie");
  const auto cookie = std::string_view(raw ? raw : "");
  const auto position = cookie.find(_cookiekey);
  if (position == std::string_view::npos) {
    _j = nlohmann::json::object();
    return;
  }

  const auto length = std::char_traits<char>::length(_cookiekey);
  const auto start = position + length;
  const auto end = cookie.find(';', start);
  const auto value = cookie.substr(start, (end == std::string_view::npos ? cookie.size() - start : end - start));

  if (nlohmann::json::accept(value)) {
    _j = nlohmann::json::parse(value);
    return;
  }

  _j = nlohmann::json::object();
  const auto script = std::format("document.cookie = '{}=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;'", _cookiekey);
  emscripten_run_script(script.c_str());
#else
  if (!std::filesystem::exists(_filename)) {
    _j = nlohmann::json::object();
    return;
  }

  std::ifstream file(_filename);
  if (nlohmann::json::accept(file)) {
    file.seekg(0);
    _j = nlohmann::json::parse(file);
    return;
  }

  _j = nlohmann::json::object();
  std::filesystem::remove(_filename);
#endif
}

void cassette::persist() const noexcept {
#ifdef EMSCRIPTEN
  constexpr auto path = "; path=/";
  const auto value = std::format("{}{}{}", _cookiekey, _j.dump(), path);
  const auto script = std::format("document.cookie = '{}';", value);
  emscripten_run_script(script.c_str());
#else
  std::ofstream file(_filename);
  file << _j.dump();
#endif
}

void cassette::clear(std::string_view key) noexcept {
  if (key.empty()) [[unlikely]] {
    return;
  }

  _j.erase(key);
  persist();
}
