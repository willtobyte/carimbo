#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <lua.hpp>
#include <physfs.h>
#include <SDL3/SDL.h>
#include <simdjson.h>
#include <sol/sol.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/static_string.hpp>
#include <boost/throw_exception.hpp>
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
#include "unmarshal.hpp"

class application;
class audiodevice;
class canvas;
class cassette;
class color;
class cursor;
class desktop;
class engine;
class enginefactory;
class entityproxy;
class envelope;
class envelopepool_impl;
class eventmanager;
class eventreceiver;
class fadeineffect;
class filesystem;
class font;
class fonteffect;
class fontfactory;
class io;
class kv;
class label;
class lifecycleobserver;
class loopable;
class mail;
class noncopyable;
class observable;
class operatingsystem;
class overlay;
class particlefactory;
class particlesystem;
class pixmap;
class pixmappool;
class postalservice;
class querybuilder;
class renderer;
class resourcemanager;
class scene;
class scenebackdrop;
class scenemanager;
class scriptengine;
class soundfx;
class soundmanager;
class statemanager;
class timermanager;
class widget;
class window;

struct keyframe;
struct bounds;
struct animation;
struct mailenvelope;
struct particle;
struct particlebatch;
struct particleprops;
struct quad;
struct timerenvelope;
struct vec2;
struct vec3;

enum class flip : int;

namespace steam { class achievement; }
