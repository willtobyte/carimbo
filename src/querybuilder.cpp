#include "querybuilder.hpp"

using namespace network;

static std::string encode(const std::string &value) {
  std::string encoded;
  encoded.reserve(value.size());
  for (char c : value) {
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else {
      encoded += '%';
      encoded += static_cast<char>((c >> 4) < 10 ? '0' + ((c >> 4) & 0xF) : 'A' + (((c >> 4) & 0xF) - 10));
      encoded += static_cast<char>((c & 0xF) < 10 ? '0' + (c & 0xF) : 'A' + ((c & 0xF) - 10));
    }
  }
  return encoded;
}

querybuilder &querybuilder::add(const std::string &key, const std::string &value) noexcept {
  _parameters.emplace(std::string(key), std::string(value));
  return *this;
}

std::string querybuilder::build() const noexcept {
  if (_parameters.empty()) {
    return {};
  }

  std::string result;
  bool first = true;
  for (const auto &[key, value] : _parameters) {
    if (!first) {
      result.push_back('&');
    } else {
      first = false;
    }
    result.append(fmt::format("{}={}", key, encode(value)));
  }
  return result;
}

std::string querybuilder::make(std::initializer_list<std::pair<std::string, std::string>> entries) {
  querybuilder builder;
  for (const auto &[key, value] : entries) {
    builder.add(key, value);
  }
  return builder.build();
}
