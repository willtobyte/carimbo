#pragma once

namespace framework {
class noncopyable {
public:
  noncopyable() = default;
  virtual ~noncopyable() = default;

  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;
  noncopyable(noncopyable&&) = default;
  noncopyable& operator=(noncopyable&&) = default;
};
}
