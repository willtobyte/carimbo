#pragma once

namespace unmarshal {
using document = simdjson::ondemand::document;
using value = simdjson::ondemand::value;
using object = simdjson::ondemand::object;
using array = simdjson::ondemand::array;

struct json final {
  simdjson::padded_string _buffer;
  simdjson::ondemand::parser _parser;
  document _document;

  explicit json(simdjson::padded_string &&buffer)
      : _buffer(std::move(buffer)), _parser{} {
    const auto error = _parser.iterate(_buffer).get(_document);
    assert(!error && "failed to parse JSON");
  }

  operator document &() noexcept { return _document; }
  document *operator->() noexcept { return &_document; }
  document &operator*() noexcept { return _document; }

  auto operator[](std::string_view key) noexcept { return _document[key]; }
  auto object() noexcept { return _document.get_object(); }
  auto array() noexcept { return _document.get_array(); }
};

[[nodiscard]] inline json parse(const std::vector<uint8_t> &data) {
  return json(simdjson::padded_string(
      reinterpret_cast<const char *>(data.data()), data.size()));
}

template <typename T>
[[nodiscard]] inline T get(auto &&source, std::string_view key) {
  T out;
  source[key].template get<T>().get(out);
  return out;
}

template <typename T>
[[nodiscard]] inline T value_or(auto &&source, std::string_view key, T fallback) {
  auto result = source[key];
  if (result.error()) [[unlikely]]
    return fallback;

  T out;
  if (result.template get<T>().get(out)) [[unlikely]]
    return fallback;

  return out;
}

[[nodiscard]] inline bool contains(auto &&source, std::string_view key) noexcept {
  return !source[key].error();
}

[[nodiscard]] inline std::optional<object> find_object(auto &&source, std::string_view key) noexcept {
  auto result = source[key];
  if (result.error()) [[unlikely]]
    return std::nullopt;

  object out;
  if (result.get_object().get(out)) [[unlikely]]
    return std::nullopt;

  return out;
}

[[nodiscard]] inline std::optional<array> find_array(auto &&source, std::string_view key) noexcept {
  auto result = source[key];
  if (result.error()) [[unlikely]]
    return std::nullopt;

  array out;
  if (result.get_array().get(out)) [[unlikely]]
    return std::nullopt;

  return out;
}

[[nodiscard]] inline std::string_view key(auto &&field) noexcept {
  std::string_view out;
  field.unescaped_key().get(out);
  return out;
}

[[nodiscard]] inline std::string_view string(auto &&element) noexcept {
  std::string_view out;
  element.get_string().get(out);
  return out;
}

[[nodiscard]] inline value value_of(auto &&element) noexcept {
  value out;
  element.get(out);
  return out;
}

[[nodiscard]] inline object object_of(auto &&element) noexcept {
  object out;
  element.get_object().get(out);
  return out;
}
}
