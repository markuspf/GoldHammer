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

StringSystem pq_group(int p, int q) {
  auto G = StringSystem(2);
  StringSystem::gen_t x=G[0], y=G[1];
  G.set_order({x}, p);
  G.set_order({y}, q);
  return G;
}

StringSystem pqr_group(int p, int q, int r) {
  auto G = StringSystem(3);
  StringSystem::gen_t x=G[0], y=G[1], z=G[2];
  G.set_order({x}, p);
  G.set_order({y}, q);
  G.set_order({z}, r);
  return G;
}

StringSystem pqrs_group(int p, int q, int r, int s) {
  auto G = StringSystem(3);
  StringSystem::gen_t x=G[0], y=G[1], z=G[2];
  G.set_order({x}, p);
  G.set_order({y}, q);
  G.set_order({z}, r);
  G.set_order({x, y, z}, s);
  return G;
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
    if(double(cg.graph.size()) * 1.10 < double(cg.graph.no_edges())) {
      return true;
    }
    printf("nodes %lu, edges %lu\n", cg.graph.size(), cg.graph.no_edges());
  }
  printf("verdict: is a tree\n");
  return false;
}

typedef enum {
  TRIANGLE,
  PQ_GROUP,
  PQR_GROUP,
  PQRS_GROUP,
  NO_GROUP_OPTIONS
} group_option;

int main(const int argc, char **argv) {
  seed_rng();

  int ugl = rand() % 10;

  char fname[80];
  uint32_t id = ugl ? uint32_t(rand()) : 0u;

  int grp = TRIANGLE;
  if(argc >= 2) {
    switch(argv[1][0]) {
      case't':grp=TRIANGLE;break;
      case'p':grp=PQ_GROUP;break;
      case'r':grp=PQR_GROUP;break;
      case's':grp=PQRS_GROUP;break;
    }
  }
  int p = (argc>=3)?atoi(argv[2]):2;
  int q = (argc>=4)?atoi(argv[3]):3;
  int r = (argc>=5)?atoi(argv[4]):7;
  int s = (argc>=6)?atoi(argv[5]):1;

  if(argc == 2) {
    sprintf(fname, "%s", argv[1]);
  } else {
    char grp_args[80];
    switch(grp){
      case TRIANGLE: sprintf(grp_args, "t-%d-%d-%d", p, q, r);break;
      case PQ_GROUP: sprintf(grp_args, "p-%d-%d", p, q);break;
      case PQR_GROUP: sprintf(grp_args, "r-%d-%d-%d", p, q, r);break;
      case PQRS_GROUP: sprintf(grp_args, "s-%d-%d-%d-%d", p, q, r, s);break;
    }
    sprintf(fname, "data/cg_system_%s_%d_%s.txt", grp_args, ugl, std::to_string(id).c_str());
  }
  std::string filename = fname;

  StringSystem t;
  do {
    switch(grp) {
      case TRIANGLE: t = triangle_group(p, q, r); break;
      case PQ_GROUP: t = pq_group(p, q); break;
      case PQR_GROUP: t = pqr_group(p, q, r); break;
      case PQRS_GROUP: t = pqrs_group(p, q, r, s); break;
    }
    t.fp_uglify(ugl);
  } while(!is_sufficient(t));
  t.to_file(filename);
  t.print();
}
