#pragma once

#define YYJSON_DISABLE_NON_STANDARD 1
#define YYJSON_DISABLE_UTF8_VALIDATION 1

#include <miniaudio.h>

#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <lua.hpp>
#include <physfs.h>
#include <SDL3/SDL.h>
#include <spng.h>
#include <yyjson.h>
#include <sol/sol.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/static_string.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#ifdef HAS_SENTRY
#include <sentry.h>
#endif

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <charconv>
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
#include <span>
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

struct quad;
struct vec2;

#include "unmarshal.hpp"

class application;
class canvas;
class cassette;
class color;
class cursor;
class desktop;
class engine;
class enginefactory;
class objectproxy;
class eventmanager;
class eventreceiver;
class fadeineffect;
class filesystem;
class font;
class fonteffect;
class fontpool;
class io;
class kv;
class label;
class lifecycleobserver;
class loopable;
class noncopyable;
class observable;
class operatingsystem;
class overlay;
class particlepool;
class pixmap;
class querybuilder;
extern SDL_Renderer* renderer;
extern ma_engine* audioengine;

class scene;
class scenebackdrop;
class scenemanager;
class scriptengine;
class soundfx;
class tilemap;
class widget;

struct keyframe;
struct bounds;
struct animation;
struct particle;
struct particlebatch;
struct glypheffect;
struct particleprops;
struct vec3;

enum class flip : int;
enum class scenekind : uint8_t;

class achievement;
class user;
