#include "cassette.hpp"

using namespace storage;

cassette::cassette() {
#ifdef EMSCRIPTEN
  const auto *raw = emscripten_run_script_string("document.cookie");
  const auto cookie = std::string(raw ? raw : "");
  const auto position = cookie.find(_cookiekey);
  if (position == std::string::npos) {
    _j = nlohmann::json::object();
    return;
  }

  const auto length = std::char_traits<char>::length(_cookiekey);
  const auto start = position + length;
  const auto end = cookie.find(';', start);
  const auto value = cookie.substr(start, (end == std::string::npos ? cookie.size() - start : end - start));

  if (nlohmann::json::accept(value)) {
    _j = nlohmann::json::parse(value);
    return;
  }

  _j = nlohmann::json::object();
  const auto script = fmt::format("document.cookie = '{}=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;'", _cookiekey);
  emscripten_run_script(script.c_str());
#else
  if (!std::filesystem::exists(_filename)) {
    _j = nlohmann::json::object();
    return;
  }

  std::ifstream file(_filename);
  const std::string content{
    std::istreambuf_iterator<char>{file},
    std::istreambuf_iterator<char>{}
  };

  if (nlohmann::json::accept(content)) {
    _j = nlohmann::json::parse(content);
    return;
  }

  _j = nlohmann::json::object();
  std::filesystem::remove(_filename);
#endif
}

void cassette::persist() const {
#ifdef EMSCRIPTEN
  constexpr auto path = "; path=/";
  const auto value = fmt::format("{}{}{}", _cookiekey, _j.dump(), path);
  const auto script = fmt::format("document.cookie = '{}';", value);
  emscripten_run_script(script.c_str());
#else
  std::ofstream file(_filename);
  file << _j.dump();
#endif
}

void cassette::clear(const std::string& key) {
  if (key.empty()) {
    return;
  }

  _j.erase(key);
  persist();
}
