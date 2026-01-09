#include "querybuilder.hpp"

static void encode_to(std::string_view value, std::string& out) {
  for (const char c : value) {
    const auto uc = static_cast<unsigned char>(c);
    if (std::isalnum(uc) || uc == '-' || uc == '_' || uc == '.' || uc == '~') {
      out.push_back(c);
      continue;
    }

    std::format_to(std::back_inserter(out), "%{:02X}", static_cast<unsigned int>(uc));
  }
}

network::querybuilder& network::querybuilder::add(std::string_view key, std::string_view value) {
  _parameters.try_emplace(key, value);
  return *this;
}

std::string network::querybuilder::build() const {
  if (_parameters.empty()) {
    return {};
  }

  std::string result;
  result.reserve(_parameters.size() * 32);
  auto first = true;
  for (const auto& [key, value] : _parameters) {
    if (!first) {
      result.push_back('&');
    }
    first = false;
    encode_to(key, result);
    result.push_back('=');
    encode_to(value, result);
  }
  return result;
}

std::string network::querybuilder::make(std::initializer_list<std::pair<std::string_view, std::string_view>> entries) {
  network::querybuilder builder;
  for (const auto& [key, value] : entries) {
    builder.add(key, value);
  }
  return builder.build();
}