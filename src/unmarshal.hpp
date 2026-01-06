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
[[nodiscard]] constexpr T read(value node) noexcept {
  if constexpr (std::same_as<T, bool>) {
    return yyjson_get_bool(node);
  } else if constexpr (std::same_as<T, float>) {
    return static_cast<float>(yyjson_get_num(node));
  } else if constexpr (std::same_as<T, double>) {
    return yyjson_get_num(node);
  } else if constexpr (std::signed_integral<T>) {
    return static_cast<T>(yyjson_get_sint(node));
  } else if constexpr (std::unsigned_integral<T>) {
    return static_cast<T>(yyjson_get_uint(node));
  } else if constexpr (std::same_as<T, std::string_view>) {
    return {yyjson_get_str(node), yyjson_get_len(node)};
  }
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
  auto result = yyjson_obj_get(node, key);
  if (!result) [[unlikely]] return fallback;
  return read<T>(result);
}

template <typename T>
[[nodiscard]] inline std::optional<T> find(value node, const char* key) noexcept {
  auto result = yyjson_obj_get(node, key);
  if (!result) [[unlikely]] return std::nullopt;
  return read<T>(result);
}

[[nodiscard]] inline std::string_view str(value node) noexcept {
  return {yyjson_get_str(node), yyjson_get_len(node)};
}

[[nodiscard]] inline size_t size(value array) noexcept {
  return yyjson_arr_size(array);
}

template <typename T>
concept decodable = requires(T& t, value v) {
  { t.decode(v) } noexcept;
};

template <typename T>
concept adl_decodable = requires(T& t, value v) {
  { decode(v, t) } noexcept;
};

template <typename T>
[[nodiscard]] inline T make(value node) noexcept {
  static_assert(decodable<T> || adl_decodable<T>, "Type must satisfy decodable or adl_decodable concept");
  [[assume(node != nullptr)]];
  T out{};
  if constexpr (decodable<T>) {
    out.decode(node);
  } else if constexpr (adl_decodable<T>) {
    decode(node, out);
  }
  return out;
}

template <typename T>
inline void into(value node, const char* key, T& out) noexcept {
  static_assert(decodable<T> || adl_decodable<T>, "Type must satisfy decodable or adl_decodable concept");
  auto result = yyjson_obj_get(node, key);
  if (!result) [[unlikely]] return;
  if constexpr (decodable<T>) {
    out.decode(result);
  } else if constexpr (adl_decodable<T>) {
    decode(result, out);
  }
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
