#pragma once

namespace unmarshal {

class json final {
  yyjson_doc* _document{};
  yyjson_val* _node{};

  [[nodiscard]] constexpr bool is_owner() const noexcept { return _document != nullptr; }

public:
  json() noexcept = default;

  constexpr json(const json& other) noexcept
      : _document(nullptr), _node(other._node) {
    assert(!other.is_owner() && "cannot copy owner json");
  }

  constexpr json& operator=(const json& other) noexcept {
    assert(!other.is_owner() && "cannot copy owner json");
    assert(!is_owner() && "cannot assign to owner json");

    _node = other._node;

    return *this;
  }

  constexpr json(json&& other) noexcept
      : _document(std::exchange(other._document, nullptr)),
        _node(std::exchange(other._node, nullptr)) {}

  constexpr json& operator=(json&& other) noexcept {
    if (this != &other) {
      if (_document) yyjson_doc_free(_document);
      _document = std::exchange(other._document, nullptr);
      _node = std::exchange(other._node, nullptr);
    }

    return *this;
  }

  constexpr json(yyjson_val* node) noexcept : _node(node) {}

  explicit json(std::span<const uint8_t> data) noexcept
      : _document(yyjson_read(reinterpret_cast<const char*>(data.data()), data.size(), YYJSON_READ_NOFLAG)),
        _node(_document ? yyjson_doc_get_root(_document) : nullptr) {
    assert(_document && "failed to parse json document");
  }

  ~json() noexcept {
    if (_document) [[likely]] yyjson_doc_free(_document);
  }

  [[nodiscard]] explicit operator bool() const noexcept {
    return _node != nullptr;
  }

  [[nodiscard]] json operator[](const char* key) const noexcept {
    return yyjson_obj_get(_node, key);
  }

  template <typename T>
  [[nodiscard]] T get() const noexcept {
    if constexpr (std::same_as<T, bool>) {
      return yyjson_get_bool(_node);
    } else if constexpr (std::same_as<T, float>) {
      return static_cast<float>(yyjson_get_num(_node));
    } else if constexpr (std::same_as<T, double>) {
      return yyjson_get_num(_node);
    } else if constexpr (std::signed_integral<T>) {
      return static_cast<T>(yyjson_get_sint(_node));
    } else if constexpr (std::unsigned_integral<T>) {
      return static_cast<T>(yyjson_get_uint(_node));
    } else if constexpr (std::same_as<T, std::string_view>) {
      return {yyjson_get_str(_node), yyjson_get_len(_node)};
    }
  }

  template <typename T>
  [[nodiscard]] T get(T fallback) const noexcept {
    if (!_node) [[unlikely]] return fallback;
    return get<T>();
  }

  [[nodiscard]] size_t size() const noexcept {
    return yyjson_arr_size(_node);
  }

  template <typename Function>
  void foreach(Function&& function) const noexcept {
    assert(_node && "foreach called on null node");

    if constexpr (std::invocable<Function, std::string_view, json>) {
      size_t index, maximum;
      yyjson_val* key;
      yyjson_val* val;
      yyjson_obj_foreach(_node, index, maximum, key, val) {
        function(std::string_view{yyjson_get_str(key), yyjson_get_len(key)}, json(val));
      }
    } else {
      size_t index, maximum;
      yyjson_val* element;
      yyjson_arr_foreach(_node, index, maximum, element) {
        function(json(element));
      }
    }
  }
};

[[nodiscard]] inline json parse(std::span<const uint8_t> data) noexcept {
  return json(data);
}

}
