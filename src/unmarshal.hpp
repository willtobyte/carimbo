#pragma once

namespace unmarshal {
using document = simdjson::ondemand::document;
using value = simdjson::ondemand::value;
using object = simdjson::ondemand::object;
using array = simdjson::ondemand::array;

class pool final {
public:
  [[nodiscard]] simdjson::ondemand::parser& acquire() noexcept {
    if (_depth >= _parsers.size()) {
      _parsers.emplace_back(std::make_unique<simdjson::ondemand::parser>());
    }

    return *_parsers[_depth++];
  }

  void release() noexcept {
    assert(_depth > 0 && "parser pool underflow");
    --_depth;
  }

  [[nodiscard]] static pool& instance() noexcept {
    thread_local pool pool;
    return pool;
  }

private:
  pool() {
    _parsers.reserve(4);
  }

  std::vector<std::unique_ptr<simdjson::ondemand::parser>> _parsers;
  size_t _depth{0};
};

struct json final {
  simdjson::padded_string _buffer;
  document _document;

  json(const json&) = delete;
  json& operator=(const json&) = delete;
  json(json&&) = delete;
  json& operator=(json&&) = delete;

  explicit json(simdjson::padded_string&& buffer)
      : _buffer(std::move(buffer)) {
    const auto error = pool::instance().acquire().iterate(_buffer).get(_document);
    assert(!error && "failed to parse JSON");
  }

  ~json() {
    pool::instance().release();
  }

  operator document&() noexcept {
    return _document;
  }

  document* operator->() noexcept {
    return &_document;
  }

  document& operator*() noexcept {
    return _document;
  }

  auto operator[](std::string_view key) noexcept {
    return _document[key];
  }

  auto object() noexcept {
    return _document.get_object();
  }

  auto array() noexcept {
    return _document.get_array();
  }
};

[[nodiscard]] inline std::unique_ptr<json> parse(const std::vector<uint8_t>& data) {
  return std::make_unique<json>(simdjson::padded_string(
      reinterpret_cast<const char*>(data.data()), data.size()));
}

template <typename T>
[[nodiscard]] inline T get(auto&& source, std::string_view key) noexcept {
  T out;
  source[key].template get<T>().get(out);
  return out;
}

template <typename T>
[[nodiscard]] inline T get(auto&& source) noexcept {
  T out;
  source.template get<T>().get(out);
  return out;
}

template <typename T>
[[nodiscard]] inline T value_or(auto&& source, std::string_view key, T fallback) noexcept {
  auto result = source[key];
  if (result.error()) [[unlikely]] {
    return fallback;
  }

  T out;
  if (result.template get<T>().get(out)) [[unlikely]] {
    return fallback;
  }

  return out;
}

[[nodiscard]] inline bool contains(auto&& source, std::string_view key) noexcept {
  return !source[key].error();
}

template <typename T>
[[nodiscard]] inline std::optional<T> find(auto&& source, std::string_view key) noexcept {
  auto result = source[key];
  if (result.error()) [[unlikely]] {
    return std::nullopt;
  }

  T out;
  if (result.template get<T>().get(out)) [[unlikely]] {
    return std::nullopt;
  }

  return out;
}

[[nodiscard]] inline std::optional<object> find_object(auto&& source, std::string_view key) noexcept {
  auto result = source[key];
  if (result.error()) [[unlikely]] {
    return std::nullopt;
  }

  object out;
  if (result.get_object().get(out)) [[unlikely]] {
    return std::nullopt;
  }

  return out;
}

[[nodiscard]] inline std::optional<array> find_array(auto&& source, std::string_view key) noexcept {
  auto result = source[key];
  if (result.error()) [[unlikely]] {
    return std::nullopt;
  }

  array out;
  if (result.get_array().get(out)) [[unlikely]] {
    return std::nullopt;
  }

  return out;
}

[[nodiscard]] inline std::string_view key(auto&& field) noexcept {
  std::string_view out;
  field.unescaped_key().get(out);
  return out;
}

[[nodiscard]] inline std::string_view string(auto&& element) noexcept {
  std::string_view out;
  element.get_string().get(out);
  return out;
}
}
