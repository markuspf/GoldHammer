#include <cstdlib>
#include <cstdint>
#include <ctime>

#include <RWS.hpp>
#include <CayleyGraph.hpp>
#include <StringSystem.hpp>


StringSystem triangle_group(int p, int q, int r) {
  auto t = StringSystem(2);
  StringSystem::gen_t x = t[0], y = t[1];
  t.set_order({x}, p);
  t.set_order({y}, q);
  t.set_order({x, y}, r);
  return t;
}

bool is_sufficient(StringSystem &ss) {
  auto cp = ss;
  int max_rules = find_max_rules_optimal(ss, 3.);
  if(max_rules == -1)return false;
  libsemigroups::RWS rws;
  rws.set_report(false);
  cp.iterate_relators([&](const auto &rel) mutable noexcept -> bool {
    auto lhs = cp.to_str(rel);
    rws.add_rule(lhs.c_str(), "");
    return true;
  });
  rws.set_max_rules(max_rules);
  rws.knuth_bendix();

  // check system is confluent
  if(rws.confluent()) {
    printf("verdict: confluent\n");
    return false;
  }

  // check that the sample of cayley graph is not a tree
  for(auto graph_size : {50, 200, 600, 1200, 1800, 3000}) {
    CayleyGraph cg(ss, rws);
    cg.traverse(graph_size, 10., 3);
    // not a tree
    if(double(cg.graph.size()) * 1.05 < double(cg.graph.no_edges())) {
      return true;
    }
    printf("nodes %lu, edges %lu\n", cg.graph.size(), cg.graph.no_edges());
  }
  printf("verdict: is a tree\n");
  return false;
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
  } while(!is_sufficient(t));
  t.to_file(filename);
  t.print();
}
