#include "application.hpp"

int main(int argc, char **argv) {
  framework::application app(argc, std::move(argv));

  return app.run();
}
