#pragma once

#include "common.hpp"

class buddy final {
public:
  buddy(uint64_t id, std::string name) noexcept;

  [[nodiscard]] uint64_t id() const noexcept;
  [[nodiscard]] const std::string& name() const noexcept;

private:
  uint64_t _id;
  std::string _name;
};

class user final {
public:
  user() noexcept = default;
  ~user() noexcept = default;

  [[nodiscard]] std::string persona() const noexcept;
  [[nodiscard]] std::vector<buddy> buddies() const noexcept;
};
