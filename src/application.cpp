#include "application.hpp"

using namespace framework;

static std::ostringstream g_stdout_capture;
static std::ostringstream g_stderr_capture;

struct capture_stream final {
  std::streambuf* old_buf;
  std::ostringstream& stream;
  capture_stream(std::ostream& os, std::ostringstream& s)
      : old_buf(os.rdbuf(s.rdbuf())), stream(s) {}
  ~capture_stream() { std::cout.rdbuf(old_buf); }
};

capture_stream capture_out(std::cout, g_stdout_capture);
capture_stream capture_err(std::cerr, g_stderr_capture);

[[noreturn]] static void fail() {
  if (const auto ptr = std::current_exception()) {
    const char* error = nullptr;

    try {
      std::rethrow_exception(ptr);
    } catch (const std::bad_exception&) {
    } catch (const std::exception& e) {
      error = e.what();
    } catch (...) {
      error = "Unhandled unknown exception";
    }

    if (error) {
      #ifdef HAVE_SENTRY
      const auto exc = sentry_value_new_exception("exception", error);
      const auto ev = sentry_value_new_event();

      auto captured_out = g_stdout_capture.str();
      auto captured_err = g_stderr_capture.str();
      sentry_value_t extras = sentry_value_new_object();
      sentry_value_set_by_key(extras, "stdout", sentry_value_new_string(captured_out.c_str()));
      sentry_value_set_by_key(extras, "stderr", sentry_value_new_string(captured_err.c_str()));
      sentry_event_set_extra(ev, "output", extras);

      sentry_event_add_exception(ev, exc);
      sentry_capture_event(ev);
      #endif

      #ifdef HAVE_STACKTRACE
      boost::stacktrace::stacktrace st;
      std::println(stderr, "Stack trace:\n{}\n", boost::stacktrace::to_string(st));
      #endif

      std::println(stderr, "{}", error);

      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Disaster", error, nullptr);

      #ifdef DEBUG
        #ifdef _MSC_VER
          __debugbreak();
        #else
          raise(SIGTRAP);
        #endif
      #endif
    }
  }

  std::exit(EXIT_FAILURE);
}

application::application(int argc, char** argv) {
  std::set_terminate(fail);

  constexpr const auto fn = [](int) {
    std::exit(EXIT_SUCCESS);
  };

  std::signal(SIGINT, fn);
  std::signal(SIGTERM, fn);

  #ifdef HAVE_SENTRY
    std::atexit([] { sentry_close(); });
  #endif

  std::atexit([] { PHYSFS_deinit(); });
  std::atexit([] { SDL_Quit(); });

  SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);
  PHYSFS_init(argv[0]);
  #ifdef HAVE_STEAM
    SteamAPI_InitSafe();
  #endif
}

int32_t application::run() {
  static_assert(std::endian::native == std::endian::little);

  const auto* p = std::getenv("CARTRIDGE");

  storage::filesystem::mount(p ? p : "cartridge.zip", "/");

  auto se = scriptengine();
  se.run();

  return 0;
}
