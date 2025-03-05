#include "cassete.hpp"

using namespace storage;

cassete::cassete() {
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
  try {
    _j = nlohmann::json::parse(value);
  } catch (...) {
    _j = nlohmann::json::object();
  }
#else
  if (!std::filesystem::exists(_filename)) {
    _j = nlohmann::json::object();
    return;
  }
  std::ifstream file(_filename);
  if (file >> _j) {
    return;
  }

  _j = nlohmann::json::object();
#endif
}

void cassete::persist() const {
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

void cassete::clear(const std::string &key) {
  if (key.empty()) {
    return;
  }

  _j.erase(key);
  persist();
}
