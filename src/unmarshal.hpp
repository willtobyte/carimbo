#pragma once

namespace unmarshal {

using value = yyjson_val*;

struct document final {
  yyjson_doc* _document;

  document(const document&) = delete;
  document& operator=(const document&) = delete;
  document(document&& other) noexcept : _document(std::exchange(other._document, nullptr)) {}
  document& operator=(document&&) = delete;

  explicit document(std::span<const uint8_t> data) noexcept
      : _document(yyjson_read(reinterpret_cast<const char*>(data.data()), data.size(), YYJSON_READ_NOFLAG)) {
    assert(_document && "failed to parse json document");
  }

  ~document() noexcept {
    yyjson_doc_free(_document);
  }

  [[nodiscard]] value operator*() const noexcept {
    return yyjson_doc_get_root(_document);
  }
};

[[nodiscard]] inline document parse(std::span<const uint8_t> data) noexcept {
  return document(data);
}

template <typename T>
[[nodiscard]] inline T read(value node) noexcept;

template <>
[[nodiscard]] inline float read<float>(value node) noexcept {
  return static_cast<float>(yyjson_get_num(node));
}

template <>
[[nodiscard]] inline double read<double>(value node) noexcept {
  return yyjson_get_num(node);
}

template <>
[[nodiscard]] inline int8_t read<int8_t>(value node) noexcept {
  return static_cast<int8_t>(yyjson_get_sint(node));
}

template <>
[[nodiscard]] inline int16_t read<int16_t>(value node) noexcept {
  return static_cast<int16_t>(yyjson_get_sint(node));
}

template <>
[[nodiscard]] inline int32_t read<int32_t>(value node) noexcept {
  return static_cast<int32_t>(yyjson_get_sint(node));
}

template <>
[[nodiscard]] inline int64_t read<int64_t>(value node) noexcept {
  return yyjson_get_sint(node);
}

template <>
[[nodiscard]] inline uint8_t read<uint8_t>(value node) noexcept {
  return static_cast<uint8_t>(yyjson_get_uint(node));
}

template <>
[[nodiscard]] inline uint16_t read<uint16_t>(value node) noexcept {
  return static_cast<uint16_t>(yyjson_get_uint(node));
}

template <>
[[nodiscard]] inline uint32_t read<uint32_t>(value node) noexcept {
  return static_cast<uint32_t>(yyjson_get_uint(node));
}

template <>
[[nodiscard]] inline uint64_t read<uint64_t>(value node) noexcept {
  return yyjson_get_uint(node);
}

template <>
[[nodiscard]] inline bool read<bool>(value node) noexcept {
  return yyjson_get_bool(node);
}

template <>
[[nodiscard]] inline std::string_view read<std::string_view>(value node) noexcept {
  return {yyjson_get_str(node), yyjson_get_len(node)};
}

[[nodiscard]] inline value child(value node, const char* key) noexcept {
  return yyjson_obj_get(node, key);
}

template <typename T>
[[nodiscard]] inline T get(value node, const char* key) noexcept {
  return read<T>(yyjson_obj_get(node, key));
}

template <typename T>
[[nodiscard]] inline T get_or(value node, const char* key, T fallback) noexcept {
  auto c = yyjson_obj_get(node, key);
  if (!c) [[unlikely]] return fallback;
  return read<T>(c);
}

template <typename T>
[[nodiscard]] inline std::optional<T> find(value node, const char* key) noexcept {
  auto c = yyjson_obj_get(node, key);
  if (!c) [[unlikely]] return std::nullopt;
  return read<T>(c);
}

[[nodiscard]] inline std::string_view str(value node) noexcept {
  return {yyjson_get_str(node), yyjson_get_len(node)};
}

[[nodiscard]] inline size_t size(value array) noexcept {
  return yyjson_arr_size(array);
}

template <typename T>
[[nodiscard]] inline T make(value node) noexcept {
  T out{};
  from_json(node, out);
  return out;
}

template <typename T>
inline bool make_into(value node, const char* key, T& out) noexcept {
  auto c = yyjson_obj_get(node, key);
  if (!c) [[unlikely]] return false;
  from_json(c, out);
  return true;
}

template <typename T, typename Container>
inline void collect(value array, Container& out) noexcept {
  if (!array) [[unlikely]] return;
  if constexpr (requires { out.reserve(size_t{}); }) {
    out.reserve(yyjson_arr_size(array));
  }
  size_t index, maximum;
  yyjson_val* element;
  yyjson_arr_foreach(array, index, maximum, element) {
    out.emplace_back(make<T>(element));
  }
}

template <typename T, typename Container>
inline void collect(value node, const char* key, Container& out) noexcept {
  collect<T>(yyjson_obj_get(node, key), out);
}

template <typename Function>
inline void foreach_object(value node, Function&& function) noexcept {
  if (!node) [[unlikely]] return;
  size_t index, maximum;
  yyjson_val* key;
  yyjson_val* val;
  yyjson_obj_foreach(node, index, maximum, key, val) {
    function(str(key), val);
  }
}

template <typename Function>
inline void foreach_array(value node, Function&& function) noexcept {
  if (!node) [[unlikely]] return;
  size_t index, maximum;
  yyjson_val* element;
  yyjson_arr_foreach(node, index, maximum, element) {
    function(element);
  }
}

}
