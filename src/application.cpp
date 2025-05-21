#include "application.hpp"

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
  PHYSFS_init(nullptr);
  #else
  PHYSFS_init(argv[0]);
  #endif
}

int32_t application::run() {
#if SANDBOX
  storage::filesystem::mount("../sandbox", "/");
#else
  #ifdef ANDROID
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject activity = SDL_AndroidGetActivity();

    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getPackageCodePath = env->GetMethodID(activityClass, "getPackageCodePath", "()Ljava/lang/String;");
    jstring jpath = static_cast<jstring>(env->CallObjectMethod(activity, getPackageCodePath));

    const char* cpath = env->GetStringUTFChars(jpath, nullptr);

    storage::filesystem::mount(cpath, "/");

    env->ReleaseStringUTFChars(jpath, cpath);
    env->DeleteLocalRef(jpath);
    env->DeleteLocalRef(activityClass);
  #else
    storage::filesystem::mount("bundle.7z", "/");
  #endif
#endif

  auto se = scriptengine();
  se.run();

  return 0;
}
