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

#ifdef STEAM
#include <steam/steam_api.h>
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <ogg/ogg.h>
#include <physfs.h>
#include <sol/sol.hpp>
#include <spng.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "constants.hpp"
#include "deleters.hpp"
#include "helpers.hpp"
#include "types.hpp"

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
class world;
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
class glyph;
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
