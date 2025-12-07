#pragma once

#include "common.hpp"

template <typename T>
class singleton {
public:
  virtual ~singleton() = default;

  static T& instance() {
    static T _instance;
    return _instance;
  }
};
