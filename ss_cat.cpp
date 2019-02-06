#include <StringSystem.hpp>


int main(int argc, char *argv[]) {
  for(int i = 1; i < argc; ++i) {
    auto ss = StringSystem::from_file(argv[i]);
    ss.print();
  }
}
