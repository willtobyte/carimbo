#pragma once

namespace unmarshal {

using value = yyjson_val*;

struct json final {
  yyjson_doc* _document;

  json(const json&) = delete;
  json& operator=(const json&) = delete;
  json(json&&) = delete;
  json& operator=(json&&) = delete;

  explicit json(const char* data, size_t length) noexcept
      : _document(yyjson_read(data, length, YYJSON_READ_NOFLAG)) {}

  ~json() noexcept {
    yyjson_doc_free(_document);
  }

  [[nodiscard]] explicit operator bool() const noexcept {
    if (!_document) [[unlikely]] return false;
    return true;
  }

  [[nodiscard]] value operator*() const noexcept {
    return yyjson_doc_get_root(_document);
  }
};

[[nodiscard]] inline json parse(const std::vector<uint8_t>& data) noexcept {
  return json(reinterpret_cast<const char*>(data.data()), data.size());
}

template <typename T>
[[nodiscard]] inline T get(value node, const char* key) noexcept;

template <>
[[nodiscard]] inline float get<float>(value node, const char* key) noexcept {
  return static_cast<float>(yyjson_get_num(yyjson_obj_get(node, key)));
}

template <>
[[nodiscard]] inline double get<double>(value node, const char* key) noexcept {
  return yyjson_get_num(yyjson_obj_get(node, key));
}

template <>
[[nodiscard]] inline int16_t get<int16_t>(value node, const char* key) noexcept {
  return static_cast<int16_t>(yyjson_get_sint(yyjson_obj_get(node, key)));
}

template <>
[[nodiscard]] inline int32_t get<int32_t>(value node, const char* key) noexcept {
  return static_cast<int32_t>(yyjson_get_sint(yyjson_obj_get(node, key)));
}

template <>
[[nodiscard]] inline int64_t get<int64_t>(value node, const char* key) noexcept {
  return yyjson_get_sint(yyjson_obj_get(node, key));
}

template <>
[[nodiscard]] inline uint64_t get<uint64_t>(value node, const char* key) noexcept {
  return yyjson_get_uint(yyjson_obj_get(node, key));
}

template <>
[[nodiscard]] inline unsigned int get<unsigned int>(value node, const char* key) noexcept {
  return static_cast<unsigned int>(yyjson_get_uint(yyjson_obj_get(node, key)));
}

template <>
[[nodiscard]] inline uint8_t get<uint8_t>(value node, const char* key) noexcept {
  return static_cast<uint8_t>(yyjson_get_uint(yyjson_obj_get(node, key)));
}

template <>
[[nodiscard]] inline bool get<bool>(value node, const char* key) noexcept {
  return yyjson_get_bool(yyjson_obj_get(node, key));
}

template <>
[[nodiscard]] inline std::string_view get<std::string_view>(value node, const char* key) noexcept {
  auto child = yyjson_obj_get(node, key);
  return {yyjson_get_str(child), yyjson_get_len(child)};
}

template <>
[[nodiscard]] inline value get<value>(value node, const char* key) noexcept {
  return yyjson_obj_get(node, key);
}

template <typename T>
[[nodiscard]] inline T get(value node) noexcept;

template <>
[[nodiscard]] inline uint64_t get<uint64_t>(value node) noexcept {
  return yyjson_get_uint(node);
}

template <>
[[nodiscard]] inline value get<value>(value node) noexcept {
  return node;
}

template <typename T>
[[nodiscard]] inline std::optional<T> find(value node, const char* key) noexcept {
  auto child = yyjson_obj_get(node, key);
  if (!child) [[unlikely]] return std::nullopt;
  if constexpr (std::is_same_v<T, value>) {
    return child;
  } else {
    return get<T>(node, key);
  }
}

template <typename T>
[[nodiscard]] inline T value_or(value node, const char* key, T fallback) noexcept {
  auto child = yyjson_obj_get(node, key);
  if (!child) [[unlikely]] return fallback;
  return get<T>(node, key);
}

[[nodiscard]] inline size_t count(value array) noexcept {
  return yyjson_arr_size(array);
}

[[nodiscard]] inline std::string_view key(value node) noexcept {
  return {yyjson_get_str(node), yyjson_get_len(node)};
}

[[nodiscard]] inline std::string_view string(value node) noexcept {
  return {yyjson_get_str(node), yyjson_get_len(node)};
}

template <typename T>
[[nodiscard]] inline T make(value node) noexcept {
  T out{};
  from_json(node, out);
  return out;
}

template <typename T>
inline bool make_if(value node, const char* key, T& out) noexcept {
  auto child = yyjson_obj_get(node, key);
  if (!child) [[unlikely]] return false;
  from_json(child, out);
  return true;
}

template <typename T, typename Container>
inline void collect(value node, const char* key, Container& out) noexcept {
  auto array = yyjson_obj_get(node, key);
  if (!array) [[unlikely]] return;
  size_t index, maximum;
  yyjson_val* element;
  yyjson_arr_foreach(array, index, maximum, element) {
    out.emplace_back(make<T>(element));
  }
}

template <typename T, typename Container>
inline void reserve(value node, const char* key, Container& out) noexcept {
  auto array = yyjson_obj_get(node, key);
  if (!array) [[unlikely]] return;
  out.reserve(yyjson_arr_size(array));
  size_t index, maximum;
  yyjson_val* element;
  yyjson_arr_foreach(array, index, maximum, element) {
    out.emplace_back(make<T>(element));
  }
}

}
