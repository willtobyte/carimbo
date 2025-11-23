#pragma once

#include "common.hpp"


class audiodevice final {
public:
  audiodevice();
  ~audiodevice() = default;

private:
  std::unique_ptr<ALCdevice, ALC_Deleter> _device;
  std::unique_ptr<ALCcontext, ALC_Deleter> _context;
};
