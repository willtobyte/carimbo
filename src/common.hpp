#pragma once

#ifdef _WIN32
#define NOMINMAX
#endif

#define WEBSOCKET

#ifdef EMSCRIPTEN
#include <emscripten.h>
#ifdef WEBSOCKET
#include <emscripten/websocket.h>
#endif
#endif

#ifndef EMSCRIPTEN
#ifdef WEBSOCKET
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#endif
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <SDL2/SDL.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <ogg/ogg.h>
#include <physfs.h>
#include <sol/sol.hpp>
#include <spng.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "constants.hpp"
#include "deleters.hpp"
#include "helpers.hpp"

namespace audio {
class audiodevice;
class soundfx;
class soundmanager;
}

namespace framework {
class application;
class engine;
class enginefactory;
class entity;
class entitymanager;
class entityprops;
class framerate;
class garbagecollector;
class lifecycleobserver;
class loopable;
class postalservice;
class resourcemanager;
class scenemanager;
class scriptengine;
class statemanager;
class timermanager;
}

namespace storage {
class cassete;
class filesystem;
}

namespace geometry {
class point;
class rect;
class size;
}

namespace graphics {
class canvas;
class effect;
class fontfactory;
class font;
class pixmap;
class pixmappool;
class overlay;
class renderer;
class widget;
class window;
}

namespace input {
class eventmanager;
class eventreceiver;
}

namespace algebra {
class vector2d;
}

namespace memory {
class kv;
}

namespace network {
class querybuilder;
class socket;
}
