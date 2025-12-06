#include "cassette.hpp"

cassette::cassette() {
#ifdef EMSCRIPTEN
  const auto* const result = emscripten_run_script_string(std::format("localStorage.getItem('{}')", _storagekey).c_str());
  const std::string_view value{result ? result : ""};

  if (value.empty() || value == "null") {
    return;
  }

  try {
    auto buffer = simdjson::padded_string(value);
    auto document = unmarshal::parse(buffer);

    for (auto field : document.object()) {
      auto key = field.unescaped_key().value();
      auto val = field.value();

      switch (val.type()) {
        case simdjson::ondemand::json_type::null:
          _data.emplace(std::string(key), nullptr);
          break;
        case simdjson::ondemand::json_type::boolean:
          _data.emplace(std::string(key), val.get_bool().value());
          break;
        case simdjson::ondemand::json_type::number: {
          auto num = val.get_number().value();
          if (num.is_int64()) {
            _data.emplace(std::string(key), num.get_int64());
          } else if (num.is_uint64()) {
            _data.emplace(std::string(key), num.get_uint64());
          } else {
            _data.emplace(std::string(key), num.get_double());
          }
          break;
        }
        case simdjson::ondemand::json_type::string:
          _data.emplace(std::string(key), std::string(val.get_string().value()));
          break;
        default:
          break;
      }
    }
  } catch (...) {
    _data.clear();
    const auto script = std::format("localStorage.removeItem('{}')", _storagekey);
    emscripten_run_script(script.c_str());
  }
#else
  if (!std::filesystem::exists(_filename)) {
    return;
  }

  try {
    auto document = unmarshal::parse(simdjson::padded_string::load(_filename));

    for (auto field : document.object()) {
      auto key = field.unescaped_key().value();
      auto val = field.value();

      switch (val.type()) {
        case simdjson::ondemand::json_type::null:
          _data.emplace(std::string(key), nullptr);
          break;
        case simdjson::ondemand::json_type::boolean:
          _data.emplace(std::string(key), val.get_bool().value());
          break;
        case simdjson::ondemand::json_type::number: {
          auto num = val.get_number().value();
          if (num.is_int64()) {
            _data.emplace(std::string(key), num.get_int64());
          } else if (num.is_uint64()) {
            _data.emplace(std::string(key), num.get_uint64());
          } else {
            _data.emplace(std::string(key), num.get_double());
          }
          break;
        }
        case simdjson::ondemand::json_type::string:
          _data.emplace(std::string(key), std::string(val.get_string().value()));
          break;
        default:
          break;
      }
    }
  } catch (...) {
    _data.clear();
    std::filesystem::remove(_filename);
  }
#endif
}

void cassette::persist() const noexcept {
  thread_local std::string buffer;
  buffer.clear();
  buffer.reserve(_data.size() * 48);
  buffer += '{';

  auto first = true;
  for (const auto& [key, value] : _data) {
    if (!first) {
      buffer += ',';
    }
    first = false;

    buffer += '"';
    for (const char c : key) {
      switch (c) {
        case '"':  buffer.append("\\\"", 2); break;
        case '\\': buffer.append("\\\\", 2); break;
        default:   buffer += c;              break;
      }
    }
    buffer.append("\":", 2);

    std::visit([](const auto& v) {
      using T = std::decay_t<decltype(v)>;
      if constexpr (std::is_same_v<T, std::nullptr_t>) {
        buffer.append("null", 4);
      } else if constexpr (std::is_same_v<T, bool>) {
        if (v) {
          buffer.append("true", 4);
        } else {
          buffer.append("false", 5);
        }
      } else if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>) {
        std::array<char, 24> num;
        const auto [ptr, ec] = std::to_chars(num.data(), num.data() + num.size(), v);
        buffer.append(num.data(), static_cast<std::size_t>(ptr - num.data()));
      } else if constexpr (std::is_same_v<T, double>) {
        std::array<char, 32> num;
        const auto [ptr, ec] = std::to_chars(num.data(), num.data() + num.size(), v);
        buffer.append(num.data(), static_cast<std::size_t>(ptr - num.data()));
      } else if constexpr (std::is_same_v<T, std::string>) {
        buffer += '"';
        for (const char c : v) {
          switch (c) {
            case '"':  buffer.append("\\\"", 2); break;
            case '\\': buffer.append("\\\\", 2); break;
            case '\b': buffer.append("\\b", 2);  break;
            case '\f': buffer.append("\\f", 2);  break;
            case '\n': buffer.append("\\n", 2);  break;
            case '\r': buffer.append("\\r", 2);  break;
            case '\t': buffer.append("\\t", 2);  break;
            default:   buffer += c;              break;
          }
        }
        buffer += '"';
      }
    }, value);
  }

  buffer += '}';

#ifdef EMSCRIPTEN
  const auto script = std::format("localStorage.setItem('{}', '{}')", _storagekey, buffer);
  emscripten_run_script(script.c_str());
#else
  auto file = std::ofstream(_filename);
  file << buffer;
#endif
}

void cassette::clear(std::string_view key) noexcept {
  if (key.empty()) [[unlikely]] {
    return;
  }

  _data.erase(std::string(key));
  persist();
}