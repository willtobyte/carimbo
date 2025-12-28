#pragma once

namespace unmarshal {
using document = simdjson::ondemand::document;
using value = simdjson::ondemand::value;
using object = simdjson::ondemand::object;
using array = simdjson::ondemand::array;

class pool final {
public:
  static constexpr size_t max_depth = 4;

  [[nodiscard]] simdjson::ondemand::parser& acquire() noexcept {
    assert(_depth < max_depth && "parser pool overflow");
    return _parsers[_depth++];
  }

  void release() noexcept {
    assert(_depth > 0 && "parser pool underflow");
    --_depth;
  }

  [[nodiscard]] static pool& instance() noexcept {
    thread_local pool p;
    return p;
  }

private:
  std::array<simdjson::ondemand::parser, max_depth> _parsers;
  size_t _depth{0};
};

struct json final {
  simdjson::padded_string _buffer;
  document _document;

  json(const json&) = delete;
  json& operator=(const json&) = delete;
  json(json&&) = default;
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

};

[[nodiscard]] inline json parse(const std::vector<uint8_t>& data) {
  return json(simdjson::padded_string(
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

static_assert(std::is_same_v<object, simdjson::ondemand::object>, "object type mismatch");
static_assert(std::is_same_v<array, simdjson::ondemand::array>, "array type mismatch");

template <typename T>
[[nodiscard]] inline std::optional<T> find(auto&& source, std::string_view key) noexcept {
  T out;
  if constexpr (std::is_same_v<T, object>) {
    if (source[key].get_object().get(out)) [[unlikely]] return std::nullopt;
  } else if constexpr (std::is_same_v<T, array>) {
    if (source[key].get_array().get(out)) [[unlikely]] return std::nullopt;
  } else {
    if (source[key].template get<T>().get(out)) [[unlikely]] return std::nullopt;
  }
  return out;
}

template <typename T>
[[nodiscard]] inline T value_or(auto&& source, std::string_view key, T fallback) noexcept {
  return find<T>(source, key).value_or(fallback);
}

[[nodiscard]] inline size_t count(array& a) noexcept {
  size_t out;
  a.count_elements().get(out);
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

template <typename T>
[[nodiscard]] inline T make(auto&& source) noexcept {
  T out{};
  value v;
  source.get(v);
  from_json(v, out);
  return out;
}

template <typename T>
inline bool make_if(auto&& source, std::string_view key, T& out) noexcept {
  auto result = source[key];
  if (result.error()) [[unlikely]] return false;
  value v;
  result.get(v);
  from_json(v, out);
  return true;
}

template <typename T, typename Container>
inline void collect(auto&& source, std::string_view key, Container& out) noexcept {
  auto result = source[key];
  if (result.error()) [[unlikely]] return;
  array a;
  if (result.get_array().get(a)) [[unlikely]] return;
  for (auto element : a) {
    out.emplace_back(make<T>(element));
  }
}

template <typename T, typename Container>
inline void reserve(auto&& source, std::string_view key, Container& out) noexcept {
  auto result = source[key];
  if (result.error()) [[unlikely]] return;
  array a;
  if (result.get_array().get(a)) [[unlikely]] return;
  size_t n;
  a.count_elements().get(n);
  a.reset();
  out.reserve(n);
  for (auto element : a) {
    out.emplace_back(make<T>(element));
  }
}
}
