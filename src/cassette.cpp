#include "cassette.hpp"

namespace {
constexpr std::string_view TYPE_NULL   = "null";
constexpr std::string_view TYPE_BOOL   = "bool";
constexpr std::string_view TYPE_INT64  = "int64";
constexpr std::string_view TYPE_UINT64 = "uint64";
constexpr std::string_view TYPE_DOUBLE = "double";
constexpr std::string_view TYPE_STRING = "string";

void encode_string_to(std::string_view str, std::string& out) {
  out.reserve(out.size() + str.size());
  for (const char c : str) {
    switch (c) {
      case '\n': out.append("\\n");  break;
      case '\r': out.append("\\r");  break;
      case '\\': out.append("\\\\"); break;
      case '\'': out.append("\\'");  break;
      default: out.push_back(c);   break;
    }
  }
}

std::string decode_string(std::string_view str) {
  if (str.find('\\') == std::string_view::npos) [[likely]] {
    return std::string(str);
  }

  std::string result;
  result.reserve(str.size());
  for (std::size_t index = 0; index < str.size(); ++index) {
    if (str[index] == '\\' && index + 1 < str.size()) {
      switch (str[index + 1]) {
        case 'n':  result += '\n'; ++index; break;
        case 'r':  result += '\r'; ++index; break;
        case '\\': result += '\\'; ++index; break;
        case '\'': result += '\''; ++index; break;
        default:   result += str[index];    break;
      }
    } else {
      result += str[index];
    }
  }

  return result;
}

bool parse(std::string_view line, std::string_view& type, std::string_view& key, std::string_view& value) {
  if (line.empty()) {
    return false;
  }

  const auto p = line.find(':');
  if (p == std::string_view::npos) {
    return false;
  }

  type = line.substr(0, p);

  const auto e = line.find('=', p + 1);
  if (e == std::string_view::npos) {
    return false;
  }

  key = line.substr(p + 1, e - p - 1);
  value = line.substr(e + 1);
  return !key.empty();
}
}

cassette::cassette() {
#ifdef EMSCRIPTEN
  const auto* const result = emscripten_run_script_string(std::format("localStorage.getItem('{}')", _storagekey).c_str());
  const std::string content{result ? result : ""};

  if (content.empty() || content == "null") {
    return;
  }
#else
  if (!std::filesystem::exists(_filename)) {
    return;
  }

  std::ifstream file(_filename, std::ios::binary | std::ios::ate);
  if (!file) {
    return;
  }

  const auto size = file.tellg();
  file.seekg(0);
  std::string content(static_cast<std::size_t>(size), '\0');
  file.read(content.data(), size);
#endif

  std::string_view remaining{content};
  while (!remaining.empty()) {
    const auto newline_pos = remaining.find('\n');
    const auto line = remaining.substr(0, newline_pos);
    remaining = (newline_pos == std::string_view::npos) ? std::string_view{} : remaining.substr(newline_pos + 1);

    if (line.empty()) {
      continue;
    }

    std::string_view type;
    std::string_view key, value;
    if (!parse(line, type, key, value)) {
      continue;
    }

    if (type == TYPE_NULL) {
      _data.try_emplace(key, nullptr);
    } else if (type == TYPE_BOOL) {
      _data.try_emplace(key, value == "1");
    } else if (type == TYPE_INT64) {
      int64_t v{};
      if (auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), v); ec == std::errc{}) {
        _data.try_emplace(key, v);
      }
    } else if (type == TYPE_UINT64) {
      uint64_t v{};
      if (auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), v); ec == std::errc{}) {
        _data.try_emplace(key, v);
      }
    } else if (type == TYPE_DOUBLE) {
      char* end{};
      const auto v = std::strtod(value.data(), &end);
      if (end != value.data() && end == value.data() + value.size()) {
        _data.try_emplace(key, v);
      }
    } else if (type == TYPE_STRING) {
      _data.try_emplace(key, decode_string(value));
    }
  }
}

void cassette::persist() const {
  std::string buffer;
  buffer.reserve(_data.size() * 64);

  for (const auto& [key, value] : _data) {
    std::visit([&key, &buffer](const auto& v) {
      using T = std::decay_t<decltype(v)>;

      if constexpr (std::is_same_v<T, std::nullptr_t>) {
        std::format_to(std::back_inserter(buffer), "{}:{}=\n", TYPE_NULL, key);
      } else if constexpr (std::is_same_v<T, bool>) {
        std::format_to(std::back_inserter(buffer), "{}:{}={}\n", TYPE_BOOL, key, v ? '1' : '0');
      } else if constexpr (std::is_same_v<T, int64_t>) {
        std::format_to(std::back_inserter(buffer), "{}:{}={}\n", TYPE_INT64, key, v);
      } else if constexpr (std::is_same_v<T, uint64_t>) {
        std::format_to(std::back_inserter(buffer), "{}:{}={}\n", TYPE_UINT64, key, v);
      } else if constexpr (std::is_same_v<T, double>) {
        std::format_to(std::back_inserter(buffer), "{}:{}={}\n", TYPE_DOUBLE, key, v);
      } else if constexpr (std::is_same_v<T, std::string>) {
        std::format_to(std::back_inserter(buffer), "{}:{}=", TYPE_STRING, key);
        encode_string_to(v, buffer);
        buffer.push_back('\n');
      }
    }, value);
  }

#ifdef EMSCRIPTEN
  std::string script;
  script.reserve(buffer.size() + 128);
  std::format_to(std::back_inserter(script), "localStorage.setItem('{}', '", _storagekey);
  encode_string_to(buffer, script);
  script.append("')");
  emscripten_run_script(script.c_str());
#else
  std::ofstream file(_filename, std::ios::binary | std::ios::trunc);
  file.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
#endif
}

void cassette::clear(std::string_view key) {
  if (key.empty()) [[unlikely]] {
    return;
  }

  _data.erase(key);
  persist();
}

void cassette::clear() {
  _data.clear();
  persist();
}

std::optional<cassette::value_type> cassette::find(std::string_view key) const noexcept {
  const auto it = _data.find(key);
  if (it == _data.end()) {
    return std::nullopt;
  }

  return it->second;
}
