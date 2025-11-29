#pragma once

class noncopyable {
public:
  noncopyable() = default;
  virtual ~noncopyable() = default;

  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;
  noncopyable(noncopyable&&) = delete;
  noncopyable& operator=(noncopyable&&) = delete;
};
