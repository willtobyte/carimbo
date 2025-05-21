#include "application.hpp"

#ifdef ANDROID
# include <android/asset_manager.h>
#endif

using namespace framework;

[[noreturn]] void fail() {
  if (const auto ptr = std::current_exception()) {
    const char* error = nullptr;

    try {
      std::rethrow_exception(ptr);
    } catch (const std::bad_exception&) {
    } catch (const std::exception &e) {
      error = e.what();
    } catch (...) {
      error = "Unhandled unknown exception";
    }

    if (error) {
      #ifdef DEBUG
        #if defined(_MSC_VER)
          __debugbreak();
        #else
          raise(SIGTRAP);
        #endif
      #endif

      #ifdef HAVE_STACKSTRACE
        boost::stacktrace::stacktrace st;
        fmt::println(stderr, "Stack trace:\n{}\n", boost::stacktrace::to_string(st));
      #endif

      fmt::println(stderr, "{}", error);

      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ink Spill Catastrophe", error, nullptr);
    }
  }

  std::exit(EXIT_FAILURE);
}

application::application(int argc, char **argv) {
  UNUSED(argc);
  UNUSED(argv);

  std::set_terminate(fail);

  constexpr const auto fn = [](int) {
    std::exit(EXIT_SUCCESS);
  };

  std::signal(SIGINT, fn);
  std::signal(SIGTERM, fn);

  std::atexit([] { PHYSFS_deinit(); });
  std::atexit([] { SDL_Quit(); });

  SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);

  #ifdef ANDROID
  PHYSFS_init(SDL_AndroidGetInternalStoragePath());
  #else
  PHYSFS_init(argv[0]);
  #endif
}

int32_t application::run() {
#if SANDBOX
  storage::filesystem::mount("../sandbox", "/");
#else
  #if ANDROID
    AAsset* asset = AAssetManager_open(g_asset_manager, "bundle.7z", AASSET_MODE_BUFFER);
    off_t length = AAsset_getLength(asset);
    const void* buffer = AAsset_getBuffer(asset);

    PHYSFS_Io* io = PHYSFS_createMemoryIo(buffer, length, false);
    PHYSFS_mountHandle(io, "bundle.7z", "/", 1);
  #else
    storage::filesystem::mount("bundle.7z", "/");
  #endif
#endif

  auto se = scriptengine();
  se.run();

  return 0;
}
