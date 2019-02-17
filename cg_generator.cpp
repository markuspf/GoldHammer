#include <iostream>
#include <type_traits>
#include <cctype>
#include <cstdlib>
#include <ctime>

#include <random>
#include <algorithm>
#include <chrono>
#include <string>

#include <libsemigroups/src/semigroups.h>
#include <libsemigroups/src/rws.h>
#include <libsemigroups/src/rwse.h>
#include <libsemigroups/src/cong.h>

#include <CayleyGraph.hpp>
#include <StringSystem.hpp>


void write_elements(std::string filename, CayleyGraph &cg) {
  auto &graph = cg.graph;
  FILE *fp = fopen(filename.c_str(), "w");
  /* FILE *fp = stdout; */
  fprintf(fp, "[");
  int line = 1;
  for(int i = 0; i < graph.size(); ++i) {
    std::string e = " ";
    e += '"';
    if(i) {
      e += graph.vertices()[i];
    } else {
      e += "<identity ...>";
    }
    e += '"';
    if(i != graph.size() - 1) {
      e += ",";
    }
    if(line + e.length() < 80) {
      fprintf(fp, "%s", e.c_str());
      line += e.length();
    } else {
      fprintf(fp, "\n%s", e.c_str());
      line = e.length();
    }
  }
  fprintf(fp, " ]\n");
  fflush(fp);
  fclose(fp);
}

void write_edges(std::string filename, CayleyGraph &cg) {
  auto &graph = cg.graph;
  FILE *fp = fopen(filename.c_str(), "w");
  /* FILE *fp = stdout; */
  fprintf(fp, "[");
  int line = 1;
  int i = 0;
  for(auto &edge : graph.edges()) {
    int j = 0;
    bool last_i = (i == graph.edges().size() - 1);
    auto &from = edge.first;
    for(auto &end : edge.second) {
      auto to = end.first;
      auto gen = end.second;
      bool last_j = (j == edge.second.size() - 1);
      std::string e = " [";
      e += std::to_string(from + 1);
      e += ", ";
      e += std::to_string(to + 1);
      e += ", ";
      e += std::to_string(gen);
      e += "]";
      if(!(last_i && last_j)) {
        e += ",";
      }
      if(line + e.length() < 80) {
        fprintf(fp, "%s", e.c_str());
        line += e.length();
      } else {
        fprintf(fp, "\n%s", e.c_str());
        line = e.length();
      }
      ++j;
    }
    ++i;
  }
  fprintf(fp, " ]\n");
  fflush(fp);
  fclose(fp);
}


void replaceAll(std::string &str, const std::string &from, const std::string &to) {
	if(from.empty())
		return;
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

void dry_run_rws(StringSystem &ss, int max_rules) {
  libsemigroups::RWS rws;
  rws.set_report(false);
  ss.iterate_relators([&](const auto &rel) mutable noexcept -> bool {
    auto lhs = ss.to_str(rel);
    rws.add_rule(lhs.c_str(), "");
    return true;
  });
  rws.set_max_rules(max_rules);
  rws.knuth_bendix();
}

int find_max_rules_optimal(StringSystem &ss, double max_dur) {
  int max_rules = 0;
  volatile double dur = 0.;
  do {
    max_rules += std::max<int>(max_rules / 8, 100);
    const auto start = now();
    dry_run_rws(ss, max_rules);
    const auto stop = now();
    dur = diff(start, stop);
    printf("rws: max_rules=%d dur=%.2f\n", max_rules, dur);
  } while(dur < max_dur);
  printf("rws: optimal max_rules=%d dur=%.2f\n", max_rules, dur);
  return max_rules;
}

int main(int argc, char *argv[]) {
  seed_rng();
  std::string fname = (argc > 1) ? argv[1] : "cg_system.txt";
  auto ss = StringSystem::from_file(fname);
  ss.print();

  int max_rules = find_max_rules_optimal(ss, 5.);
  libsemigroups::RWS rws;
  rws.set_report(false);
  ss.iterate_relators([&](const auto &rel) mutable noexcept -> bool {
    auto lhs = ss.to_str(rel);
    rws.add_rule(lhs.c_str(), "");
    return true;
  });
  rws.set_max_rules(max_rules);
  rws.knuth_bendix();
  printf("rws: completed\n");

  // memoize a part of cayley graph
  auto cg = CayleyGraph(ss, rws);
  cg.traverse(600, 120.);

  std::string elems = fname, edges = fname;
	replaceAll(elems, "cg_system", "cg_elements");
	replaceAll(edges, "cg_system", "cg_edges");
  write_elements(elems, cg);
  write_edges(edges, cg);
}
