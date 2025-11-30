#include "cassette.hpp"

cassette::cassette() {
#ifdef EMSCRIPTEN
  const auto* const result = emscripten_run_script_string(std::format("localStorage.getItem('{}')", _storagekey).c_str());
  const auto value{result ? result : ""};

  if (value.empty() || value == "null") {
    _j = nlohmann::json::object();
    return;
  }

  if (nlohmann::json::accept(value)) {
    _j = nlohmann::json::parse(value);
    return;
  }

  _j = nlohmann::json::object();
  const auto script = std::format("localStorage.removeItem('{}')", _storagekey);
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
  const auto script = std::format("localStorage.setItem('{}', '{}')", _storagekey, _j.dump());
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
