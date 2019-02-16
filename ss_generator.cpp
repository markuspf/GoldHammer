#include <cstdlib>
#include <cstdint>
#include <ctime>


#include <StringSystem.hpp>


StringSystem triangle_group(int p, int q, int r) {
  auto t = StringSystem(2);
  StringSystem::gen_t x = t[0], y = t[1];
  t.set_order({x}, p);
  t.set_order({y}, q);
  t.set_order({x, y}, r);
  return t;
}

int main(const int argc, char **argv) {
  seed_rng();

  int p = 2, q = 3, r = 7;
  int ugl = rand() % 7;
  char fname[80];
  uint32_t id = ugl ? uint32_t(rand()) : 0u;
  if(argc >= 4) {
    p = atoi(argv[1]), q = atoi(argv[2]), r = atoi(argv[3]);
  }
  if(argc == 2) {
    sprintf(fname, "%s", argv[1]);
  } else {
    sprintf(fname, "data/cg_system_%d-%d-%d_%d_%s.txt", p, q, r, ugl, std::to_string(id).c_str());
  }
  std::string filename = fname;

  auto t = triangle_group(p, q, r);
  t.fp_uglify(ugl);
  t.to_file(filename);
  t.print();
}
