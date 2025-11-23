#pragma once

#include "common.hpp"

class resourcemanager final {
public:
  resourcemanager() = delete;
  resourcemanager(
    std::shared_ptr<::renderer> renderer,
    std::shared_ptr<::audiodevice> audiodevice,
    std::shared_ptr<::engine> engine
  );

  ~resourcemanager() = default;

  void flush();

  void update(float delta);

  void prefetch();
  void prefetch(const std::vector<std::string>& filenames);

#ifndef NDEBUG
  void debug() const;
#endif

  std::shared_ptr<::renderer> renderer() const;
  std::shared_ptr<::pixmappool> pixmappool() const;
  std::shared_ptr<::soundmanager> soundmanager() const;
  std::shared_ptr<::fontfactory> fontfactory() const;

private:
  std::shared_ptr<::renderer> _renderer;
  std::shared_ptr<::audiodevice> _audiodevice;
  std::shared_ptr<::engine> _engine;
  std::shared_ptr<::pixmappool> _pixmappool;
  std::shared_ptr<::soundmanager> _soundmanager;
  std::shared_ptr<::fontfactory> _fontfactory;
};
