#include "querybuilder.hpp"

static std::string encode(std::string_view value) {
  std::string encoded;
  encoded.reserve(value.size() * 3);

  for (char c : value) {
    const unsigned char uc = static_cast<unsigned char>(c);
    if (std::isalnum(uc) || uc == '-' || uc == '_' || uc == '.' || uc == '~') {
      encoded.push_back(c);
      continue;
    }

    std::format_to(std::back_inserter(encoded), "%{:02X}", static_cast<unsigned int>(uc));
  }

  return encoded;
}

network::querybuilder& network::querybuilder::add(std::string_view key, std::string_view value) {
  _parameters.emplace(key, value);
  return *this;
}

std::string
network::querybuilder::build() const {
  if (_parameters.empty()) {
    return {};
  }

  std::string result;
  result.reserve(_parameters.size() * 32);
  bool first = true;
  for (const auto& [key, value] : _parameters) {
    if (!first) {
      result += '&';
    } else {
      first = false;
    }

    std::format_to(std::back_inserter(result), "{}={}", key, encode(value));
  }
  return result;
}

std::string
network::querybuilder::make(std::initializer_list<std::pair<std::string, std::string>> entries) {
  network::querybuilder builder;
  for (const auto& [key, value] : entries) {
    builder.add(key, value);
  }
  return builder.build();
}