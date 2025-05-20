#include "application.hpp"

const unsigned char null_terminated_file_data[] = {
#embed "main.cpp"
};

int main(int argc, char **argv) {
  framework::application app(argc, std::move(argv));

  return app.run();
}
