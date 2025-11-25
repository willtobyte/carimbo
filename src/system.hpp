#pragma once

#include "common.hpp"

#include "entity.hpp"

class system {
public:
  std::unordered_set<entity> entities;
};
