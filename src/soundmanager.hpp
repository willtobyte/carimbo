#pragma once

#include "common.hpp"

class soundmanager final {
public:
  soundmanager(std::string_view scenename);
  ~soundmanager() noexcept;

  void add(std::string_view name);

  void populate(sol::table& pool) const;

  void stop() const noexcept;

  void clear();

  void update(float delta);

private:
  std::string _scenename;
  boost::unordered_flat_map<std::string, std::shared_ptr<soundfx>, transparent_string_hash, std::equal_to<>> _sounds;
};
