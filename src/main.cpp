#include "application.hpp"


int main(int argc, char** argv) {
  static const char data[] = {
    #embed __FILE__ limit(32)
  };

  UNUSED(data);

  framework::application app(argc, argv);
  return app.run();
}

#ifdef ANDROID
#include <android/native_activity.h>

extern "C" __attribute__((visibility("default"))) void ANativeActivity_onCreate(ANativeActivity*, void*, size_t) {
  static constexpr int argc = 1;
  static constexpr char arg0[] = "android_app";
  static char* argv[] = { const_cast<char*>(arg0), nullptr };

  framework::application app(argc, argv);
  app.run();
}
#endif
