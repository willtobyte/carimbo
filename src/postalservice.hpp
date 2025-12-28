#pragma once

#include "common.hpp"

class envelope;

struct mail final {
  uint64_t to;
  uint64_t from;
  std::string body;

  mail(
    std::shared_ptr<entityproxy> to,
    std::shared_ptr<entityproxy> from,
    std::string_view body
  )
    : to(to->id()),
      from(from->id()),
      body(body) {}
};

class postalservice final {
public:
  postalservice();

  void post(const mail& message);

private:
  envelopepool_impl& _envelopepool;
};
