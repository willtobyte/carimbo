#include "application.hpp"

int main(int argc, char** argv) {
  framework::application app(argc, argv);
  return app.run();
}

#if defined(__ANDROID__)
#include <android/native_activity.h>

extern "C" void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
  UNUSED(activity);
  UNUSED(savedState);
  UNUSED(savedStateSize);

  static constexpr int argc = 1;
  static constexpr char arg0[] = "android_app";
  static char* argv[] = { const_cast<char*>(arg0), nullptr };

  framework::application app(argc, argv);
  app.run();
}
#endif
