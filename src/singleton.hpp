#pragma once

#include "common.hpp"

namespace framework {
template <typename T>
class singleton {
public:
  virtual ~singleton() = default;

  static std::shared_ptr<T> instance() {
    static std::once_flag _flag;
    static std::shared_ptr<T> _instance;

    std::call_once(_flag, []() {
      _instance = std::make_shared<T>();
    });

    return _instance;
  }
};
}
