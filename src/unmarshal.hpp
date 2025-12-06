#pragma once

namespace unmarshal {
using document = simdjson::ondemand::document;
using value = simdjson::ondemand::value;
using object = simdjson::ondemand::object;
using array = simdjson::ondemand::array;

struct parsed final {
  simdjson::padded_string _buffer;
  simdjson::ondemand::parser _parser;
  document _document;

  parsed(simdjson::padded_string&& buffer)
      : _buffer(std::move(buffer)), _parser{}, _document(_parser.iterate(_buffer)) {}

  operator document&() noexcept { return _document; }
  document* operator->() noexcept { return &_document; }
  document& operator*() noexcept { return _document; }

  auto operator[](std::string_view key) noexcept { return _document[key]; }
  auto object() noexcept { return _document.get_object(); }
  auto array() noexcept { return _document.get_array(); }
};

namespace detail {
[[nodiscard]] inline simdjson::padded_string to_padded_string(const std::vector<uint8_t>& data) {
  return simdjson::padded_string(reinterpret_cast<const char*>(data.data()), data.size());
}
}

[[nodiscard]] inline parsed parse(const std::vector<uint8_t>& data) {
  return parsed(detail::to_padded_string(data));
}

template <typename T>
[[nodiscard]] inline T get(object json, std::string_view key) {
  return json[key].template get<T>();
}

template <typename T>
[[nodiscard]] inline T get(value json, std::string_view key) {
  return json[key].template get<T>();
}

template <typename T>
[[nodiscard]] inline T get(document& document, std::string_view key) {
  return document[key].template get<T>();
}

template <typename T>
[[nodiscard]] inline T value_or(object json, std::string_view key, T fallback) {
  auto result = json[key];
  if (result.error()) [[unlikely]] {
    return fallback;
  }

  auto typed = result.template get<T>();
  if (typed.error()) [[unlikely]] {
    return fallback;
  }

  return typed.value();
}

template <typename T>
[[nodiscard]] inline T value_or(value json, std::string_view key, T fallback) {
  auto result = json[key];
  if (result.error()) [[unlikely]] {
    return fallback;
  }

  auto typed = result.template get<T>();
  if (typed.error()) [[unlikely]] {
    return fallback;
  }

  return typed.value();
}

template <typename T>
[[nodiscard]] inline T value_or(document& document, std::string_view key, T fallback) {
  auto result = document[key];
  if (result.error()) [[unlikely]] {
    return fallback;
  }

  auto typed = result.template get<T>();
  if (typed.error()) [[unlikely]] {
    return fallback;
  }

  return typed.value();
}

[[nodiscard]] inline bool contains(value json, std::string_view key) noexcept {
  return !json[key].error();
}

[[nodiscard]] inline std::optional<object> find_object(value json, std::string_view key) noexcept {
  auto result = json[key];
  if (result.error()) [[unlikely]] {
    return std::nullopt;
  }

  auto typed = result.get_object();
  if (typed.error()) [[unlikely]] {
    return std::nullopt;
  }

  return typed.value();
}

[[nodiscard]] inline std::optional<object> find_object(object object, std::string_view key) noexcept {
  auto result = object[key];
  if (result.error()) [[unlikely]] {
    return std::nullopt;
  }

  auto typed = result.get_object();
  if (typed.error()) [[unlikely]] {
    return std::nullopt;
  }

  return typed.value();
}

[[nodiscard]] inline std::optional<object> find_object(parsed& p, std::string_view key) noexcept {
  auto result = p[key];
  if (result.error()) [[unlikely]] {
    return std::nullopt;
  }

  auto typed = result.get_object();
  if (typed.error()) [[unlikely]] {
    return std::nullopt;
  }

  return typed.value();
}

[[nodiscard]] inline std::optional<array> find_array(value json, std::string_view key) noexcept {
  auto result = json[key];
  if (result.error()) [[unlikely]] {
    return std::nullopt;
  }

  auto typed = result.get_array();
  if (typed.error()) [[unlikely]] {
    return std::nullopt;
  }

  return typed.value();
}

[[nodiscard]] inline std::optional<array> find_array(document& document, std::string_view key) noexcept {
  auto result = document[key];
  if (result.error()) [[unlikely]] {
    return std::nullopt;
  }

  auto typed = result.get_array();
  if (typed.error()) [[unlikely]] {
    return std::nullopt;
  }

  return typed.value();
}
}