#include "enginefactory.hpp"

#include "canvas.hpp"
#include "engine.hpp"
#include "eventmanager.hpp"
#include "fontpool.hpp"
#include "scenemanager.hpp"

enginefactory& enginefactory::with_title(const std::string_view title) {
  _title = title;
  return *this;
}

enginefactory& enginefactory::with_width(const int width) noexcept {
  _width = width;
  return *this;
}

enginefactory& enginefactory::with_height(const int height) noexcept {
  _height = height;
  return *this;
}

enginefactory& enginefactory::with_scale(const float scale) noexcept {
  _scale = scale;
  return *this;
}

enginefactory& enginefactory::with_gravity(const float gravity) noexcept {
  _gravity = gravity;
  return *this;
}

enginefactory& enginefactory::with_fullscreen(const bool fullscreen) noexcept {
  _fullscreen = fullscreen;
  return *this;
}

enginefactory& enginefactory::with_sentry(const std::string_view dsn) {
  if (dsn.empty()) {
    return *this;
  }

#ifdef NDEBUG
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

#ifdef HAS_SENTRY
  auto* options = sentry_options_new();
  sentry_options_set_debug(options, true);
  sentry_options_set_dsn(options, dsn.data());
  sentry_options_set_sample_rate(options, 1.0);

#ifdef _WIN32
  static constexpr auto program = "crashpad_handler.exe";
#else
  static constexpr auto program = "crashpad_handler";
#endif

  const auto crashpad = std::filesystem::current_path() / program;
  const auto handler_path = crashpad.string();
  sentry_options_set_handler_path(options, handler_path.c_str());

  sentry_options_set_database_path(options, ".sentry");

  sentry_options_add_attachment(options, "cassette.tape");
  sentry_options_add_attachment(options, "stdout.txt");
  sentry_options_add_attachment(options, "stderr.txt");
  sentry_options_add_attachment(options, "VERSION");

  sentry_init(options);
  std::at_quick_exit([] { sentry_close(); });
#endif
#endif

  return *this;
}

enginefactory& enginefactory::with_ticks(const uint8_t ticks) noexcept {
  _ticks = ticks;
  return *this;
}

std::shared_ptr<engine> enginefactory::create() const {
  const auto engine = std::make_shared<::engine>();
  const auto window = std::make_shared<::window>(_title, _width, _height, _fullscreen);

  const auto vsync = std::getenv("NOVSYNC") ? 0 : 1;
  const auto props = SDL_CreateProperties();
  SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, static_cast<SDL_Window*>(*window));
  SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, vsync);
  SDL_SetStringProperty(props, SDL_PROP_RENDERER_CREATE_NAME_STRING, nullptr);

  renderer = SDL_CreateRendererWithProperties(props);

  SDL_DestroyProperties(props);

  std::at_quick_exit([] { SDL_DestroyRenderer(renderer); });

  SDL_SetRenderLogicalPresentation(renderer, _width, _height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_SetRenderScale(renderer, _scale, _scale);

  const auto eventmanager = std::make_shared<::eventmanager>();
  const auto fontpool = std::make_shared<::fontpool>();
  const auto overlay = std::make_shared<::overlay>(eventmanager);
  const auto scenemanager = std::make_shared<::scenemanager>();
  const auto canvas = std::make_shared<::canvas>();

  engine->set_eventmanager(eventmanager);
  engine->set_scenemanager(scenemanager);
  engine->set_window(window);
  engine->set_overlay(overlay);
  engine->set_canvas(canvas);
  engine->set_ticks(_ticks);

  eventmanager->add_receiver(engine);
  eventmanager->add_receiver(overlay);
  eventmanager->add_receiver(scenemanager);

  overlay->set_fontpool(fontpool);
  scenemanager->set_fontpool(fontpool);

  return engine;
}
