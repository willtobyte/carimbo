#pragma once

namespace unmarshal {
using document = simdjson::ondemand::document;
using value = simdjson::ondemand::value;
using object = simdjson::ondemand::object;
using array = simdjson::ondemand::array;

namespace dom {
using element = simdjson::dom::element;

struct json final {
  simdjson::dom::parser _parser;
  element _document;

  explicit json(const std::vector<uint8_t> &data) {
    _document = _parser.parse(reinterpret_cast<const char *>(data.data()), data.size());
  }

  element operator[](std::string_view key) const { return _document[key]; }
};

[[nodiscard]] inline json parse(const std::vector<uint8_t> &data) {
  return json(data);
}

template <typename T>
[[nodiscard]] inline T get(element source, std::string_view key) {
  return static_cast<T>(source[key]);
}

template <typename T>
[[nodiscard]] inline T value_or(element source, std::string_view key, T fallback) {
  auto result = source[key];
  if (result.error()) [[unlikely]]
    return fallback;
  return static_cast<T>(result.value());
}

template <typename T>
[[nodiscard]] inline T value_or(simdjson::simdjson_result<element> source, T fallback) {
  if (source.error()) [[unlikely]]
    return fallback;
  if constexpr (std::is_same_v<T, float>) {
    return static_cast<float>(double(source.value()));
  } else if constexpr (std::is_same_v<T, unsigned int>) {
    return static_cast<unsigned int>(uint64_t(source.value()));
  } else if constexpr (std::is_same_v<T, size_t>) {
    return static_cast<size_t>(uint64_t(source.value()));
  } else {
    return static_cast<T>(source.value());
  }
}

template <typename T>
[[nodiscard]] inline std::pair<T, T> range(element object, std::string_view key, T default_start, T default_end) {
  auto nested = object[key];
  if (nested.error()) [[unlikely]]
    return {default_start, default_end};

  return {
    value_or<T>(nested.value()["start"], default_start),
    value_or<T>(nested.value()["end"], default_end)};
  }
}

struct json final {
  simdjson::padded_string _buffer;
  simdjson::ondemand::parser _parser;
  document _document;

  explicit json(simdjson::padded_string &&buffer)
      : _buffer(std::move(buffer)), _parser{} {
    _parser.iterate(_buffer).get(_document);
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

template <typename T>
[[nodiscard]] inline std::pair<T, T> range(std::optional<object> opt, std::string_view key, T default_start, T default_end) noexcept {
  if (!opt) [[unlikely]]
    return {default_start, default_end};

  auto nested = find_object(*opt, key);
  if (!nested) [[unlikely]]
    return {default_start, default_end};

  return {
      value_or(*nested, "start", default_start),
      value_or(*nested, "end", default_end)};
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
