#include <StringSystem.hpp>


int main(const int argc, const char *argv[]) {
  seed_rng();
  const char *filename = "cg_system.txt";
  if(argc >= 2)filename = argv[1];
  decltype(auto) ss = StringSystem::from_file(filename);
  ss.fp_add_shortcut(ss.random_word(5));
  ss.fp_uglify_add_generator();
  ss.fp_add_shortcut(ss.random_word(5));
  ss.fp_uglify_add_generator();
  ss.print();
}
