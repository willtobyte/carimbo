#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <box2d/box2d.h>
#include <lua.hpp>
#include <nlohmann/json.hpp>
#include <physfs.h>
#include <SDL3/SDL.h>
#include <sol/sol.hpp>

#ifdef HAS_SENTRY
#include <sentry.h>
#endif

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <numbers>
#include <optional>
#include <print>
#include <random>
#include <ranges>
#include <stdexcept>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "constant.hpp"
#include "defer.hpp"
#include "helper.hpp"

namespace audio {
  class audiodevice;
  class soundfx;
  class soundmanager;
}

namespace framework {
  class application;
  class engine;
  class enginefactory;
  class lifecycleobserver;
  class loopable;
  class object;
  class objectmanager;
  class resourcemanager;
  class postalservice;
  class scene;
  class scenemanager;
  class scriptengine;
  class statemanager;
  class timermanager;
  class world;
  class querybuilder;
  struct mailenvelope;
  struct timerenvelope;
  class envelope;
}

namespace geometry {
  class point;
  class rectangle;
  class size;
  class margin;
}

namespace graphics {
  class canvas;
  class color;
  class cursor;
  class font;
  class fonteffect;
  class fontfactory;
  class label;
  class overlay;
  class particlesystem;
  class pixmap;
  class pixmappool;
  class renderer;
  class tilemap;
  class window;
  enum class reflection : int;
}

namespace input {
  class eventmanager;
  class eventreceiver;
}

namespace storage {
  class cassette;
  class filesystem;
}

namespace memory {
  class kv;
}

namespace platform {
  class desktop;
  class operatingsystem;
}

namespace steam {
  class achievement;
}

namespace i18n {
  class locales;
}

namespace widget {
  class widget;
}
