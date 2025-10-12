#include "querybuilder.hpp"

using namespace network;

static std::string encode(const std::string& value) {
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

querybuilder& querybuilder::add(const std::string& key, const std::string& value) {
  _parameters.emplace(std::string(key), std::string(value));
  return *this;
}

std::string querybuilder::build() const {
  if (_parameters.empty()) {
    return {};
  }

  std::string result;
  bool first = true;
  for (const auto& [key, value] : _parameters) {
    if (!first) {
      result.push_back('&');
    } else {
      first = false;
    }

    result.append(std::format("{}={}", key, encode(value)));
  }
  return result;
}

std::string querybuilder::make(std::initializer_list<std::pair<std::string, std::string>> entries) {
  querybuilder builder;
  for (const auto& [key, value] : entries) {
    builder.add(key, value);
  }
  return builder.build();
}
