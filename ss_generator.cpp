#include <cstdlib>
#include <cstdint>
#include <ctime>

#include <libsemigroups/src/semigroups.h>
#include <libsemigroups/src/rws.h>
#include <libsemigroups/src/rwse.h>
#include <libsemigroups/src/cong.h>

#include <StringSystem.hpp>


StringSystem triangle_group(int p, int q, int r) {
  auto t = StringSystem(2);
  StringSystem::gen_t x = t[0], y = t[1];
  t.set_order({x}, p);
  t.set_order({y}, q);
  t.set_order({x, y}, r);
  return t;
}

bool is_confluent(StringSystem &ss) {
  auto cp = ss;
  libsemigroups::RWS rws;
  rws.set_report(false);
  cp.iterate_relators([&](const auto &rel) mutable noexcept -> bool {
    auto lhs = cp.to_str(rel);
    rws.add_rule(lhs.c_str(), "");
    return true;
  });
  rws.set_max_rules(400);
  rws.knuth_bendix();
  return rws.confluent();
}

int main(const int argc, char **argv) {
  seed_rng();

  int p = 2, q = 3, r = 7;
  int ugl = rand() % 10;
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

  StringSystem t;
  do {
    t = triangle_group(p, q, r);
    t.fp_uglify(ugl);
  } while(is_confluent(t));
  t.to_file(filename);
  t.print();
}
