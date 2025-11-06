#include "enginefactory.hpp"

#include "audiodevice.hpp"
#include "engine.hpp"
#include "eventmanager.hpp"
#include "objectmanager.hpp"
#include "particlesystem.hpp"
#include "resourcemanager.hpp"
#include "postalservice.hpp"
#include "scenemanager.hpp"
#include "timermanager.hpp"
#include "window.hpp"

using namespace framework;

enginefactory& enginefactory::with_title(const std::string& title) {
  _title = title;
  return *this;
}

enginefactory& enginefactory::with_width(int32_t width) {
  _width = width;
  return *this;
}

enginefactory& enginefactory::with_height(int32_t height) {
  _height = height;
  return *this;
}

enginefactory& enginefactory::with_scale(float scale) {
  _scale = scale;
  return *this;
}

enginefactory& enginefactory::with_gravity(float gravity) {
  _gravity = gravity;
  return *this;
}

enginefactory& enginefactory::with_fullscreen(bool fullscreen) {
  _fullscreen = fullscreen;
  return *this;
}

enginefactory& enginefactory::with_sentry(const std::string& dsn) {
  if (dsn.empty()) {
    return *this;
  }

#ifdef EMSCRIPTEN
  static constexpr auto script = R"javascript(
    (function(){{
      const __dsn="{}";
      if (window.Sentry && window.__sentry_inited__) return;
      const script = document.createElement('script');
      script.src = 'https://cdn.jsdelivr.net/npm/@sentry/browser@latest/build/bundle.min.js';
      script.crossOrigin = 'anonymous';
      script.defer = true;
      script.onload = function(){{
        if (!window.Sentry) return;
        window.Sentry.init({{ dsn: __dsn }});
        window.__sentry_inited__ = true;
      }};
      document.head.appendChild(script);
    }})();
  )javascript";

  emscripten_run_script(std::format(script, dsn).c_str());
#endif

#ifdef HAVE_SENTRY
  auto* options = sentry_options_new();
  sentry_options_set_debug(options, true);
  sentry_options_set_enable_logs(options, true);
  sentry_options_set_dsn(options, dsn.c_str());
  sentry_options_set_sample_rate(options, 1.0);

  sentry_options_set_handler_path(options,
#ifdef _WIN32
    "crashpad_handler.exe"
#else
    "crashpad_handler"
#endif
  );

  sentry_options_set_database_path(options, ".sentry");

  sentry_options_add_attachment(options, "cassette.tape");
  sentry_options_add_attachment(options, "stdout.txt");
  sentry_options_add_attachment(options, "stderr.txt");
  sentry_options_add_attachment(options, "VERSION");
  sentry_init(options);
#endif

  return *this;
}

std::shared_ptr<engine> enginefactory::create() const {
  const auto audiodevice = std::make_shared<audio::audiodevice>();
  const auto engine = std::make_shared<framework::engine>();
  const auto window = std::make_shared<graphics::window>(_title, _width, _height, _fullscreen);
  const auto renderer = window->create_renderer(_scale);
  const auto eventmanager = std::make_shared<input::eventmanager>(renderer);
  const auto resourcemanager = std::make_shared<framework::resourcemanager>(renderer, audiodevice, engine);
  const auto postalservice = std::make_shared<framework::postalservice>();
  const auto overlay = std::make_shared<graphics::overlay>(resourcemanager, eventmanager);
  const auto statemanager = std::make_shared<framework::statemanager>();
  const auto objectmanager = std::make_shared<framework::objectmanager>();
  const auto particlesystem = std::make_shared<graphics::particlesystem>(resourcemanager);
  const auto timermanager = std::make_shared<framework::timermanager>();
  const auto scenemanager = std::make_shared<framework::scenemanager>(resourcemanager, objectmanager, particlesystem, timermanager);
  const auto world = std::make_shared<framework::world>(renderer);

  engine->set_audiodevice(audiodevice);
  engine->set_objectmanager(objectmanager);
  engine->set_eventmanager(eventmanager);
  engine->set_overlay(overlay);
  engine->set_renderer(renderer);
  engine->set_resourcemanager(resourcemanager);
  engine->set_scenemanager(scenemanager);
  engine->set_statemanager(statemanager);
  engine->set_particlesystem(particlesystem);
  engine->set_postalservice(postalservice);
  engine->set_timermanager(timermanager);
  engine->set_window(window);
  engine->set_world(world);

  eventmanager->add_receiver(engine->objectmanager());
  eventmanager->add_receiver(engine);
  eventmanager->add_receiver(engine->statemanager());
  eventmanager->add_receiver(overlay);
  eventmanager->add_receiver(scenemanager);

  objectmanager->set_resourcemanager(resourcemanager);
  objectmanager->set_scenemanager(scenemanager);
  objectmanager->set_world(world);

  world->set_objectmanager(objectmanager);

  return engine;
}
