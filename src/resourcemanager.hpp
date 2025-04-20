#pragma once

#include "common.hpp"

#include "fontfactory.hpp"
#include "pixmappool.hpp"
#include "soundmanager.hpp"

namespace framework {
class resourcemanager final {
public:
  resourcemanager() = delete;
  resourcemanager(std::shared_ptr<graphics::renderer> renderer, std::shared_ptr<audio::audiodevice> audiodevice);
  ~resourcemanager() = default;

  void flush();

  void prefetch();
  void prefetch(const std::vector<std::string> &filenames);

  std::shared_ptr<graphics::renderer> renderer() const;
  std::shared_ptr<graphics::pixmappool> pixmappool() const;
  std::shared_ptr<audio::soundmanager> soundmanager() const;
  std::shared_ptr<graphics::fontfactory> fontfactory() const;

private:
  std::shared_ptr<graphics::renderer> _renderer;
  std::shared_ptr<audio::audiodevice> _audiodevice;
  std::shared_ptr<graphics::pixmappool> _pixmappool;
  std::shared_ptr<audio::soundmanager> _soundmanager;
  std::shared_ptr<graphics::fontfactory> _fontfactory;
};
}
