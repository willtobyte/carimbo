#pragma once

#include "common.hpp"

#include "eventmanager.hpp"
#include "fontfactory.hpp"
#include "pixmappool.hpp"
#include "soundmanager.hpp"

namespace framework {
class resourcemanager final {
public:
  resourcemanager() = delete;
  resourcemanager(
    std::shared_ptr<graphics::renderer> renderer,
    std::shared_ptr<audio::audiodevice> audiodevice,
    std::shared_ptr<input::eventmanager> eventmanager
  );

  ~resourcemanager() noexcept = default;

  void flush() noexcept;

  void prefetch();
  void prefetch(const std::vector<std::string>& filenames);

  #ifdef DEBUG
  void debug() const noexcept;
  #endif

  std::shared_ptr<graphics::renderer> renderer() const noexcept;
  std::shared_ptr<graphics::pixmappool> pixmappool() const noexcept;
  std::shared_ptr<audio::soundmanager> soundmanager() const noexcept;
  std::shared_ptr<graphics::fontfactory> fontfactory() const noexcept;

private:
  std::shared_ptr<graphics::renderer> _renderer;
  std::shared_ptr<audio::audiodevice> _audiodevice;
  std::shared_ptr<input::eventmanager> _eventmanager;
  std::shared_ptr<graphics::pixmappool> _pixmappool;
  std::shared_ptr<audio::soundmanager> _soundmanager;
  std::shared_ptr<graphics::fontfactory> _fontfactory;
};
}
