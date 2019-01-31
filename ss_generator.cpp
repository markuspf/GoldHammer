#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>


#include <StringSystem.hpp>


StringSystem triangle_group(int p, int q, int r) {
  auto t = StringSystem(2);
  StringSystem::gen_t x = t[0], y = t[1];
  t.set_order({x}, p);
  t.set_order({y}, q);
  t.set_order({x, y}, r);
  return t;
}

void seed_rng() {
  int fd=open("/dev/urandom", O_RDONLY);
  if(fd==-1) abort();
  unsigned seed,pos = 0;
  while(pos<sizeof(seed)){int a=read(fd,(char*)&seed+pos,sizeof(seed)-pos);if(a<=0)abort();pos += a;}
  srand(seed);
  close(fd);
}

int main(const int argc, char **argv) {
  seed_rng();

  int p = 2, q = 3, r = 7;
  if(argc >= 4) {
    p = atoi(argv[1]), q = atoi(argv[2]), r = atoi(argv[3]);
  }
  uint64_t id = (uint64_t(rand()) << 31) | uint64_t(rand());
  char fname[80];
  sprintf(fname, "cg_system_%d-%d-%d_%s.txt", p, q, r, std::to_string(id).c_str());
  std::string filename = fname;

  auto t = triangle_group(p, q, r);
  t.fp_uglify(2);
  t.to_file(filename);
  t.print();
}
