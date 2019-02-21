#include <iostream>
#include <type_traits>
#include <cctype>
#include <cstdlib>
#include <ctime>

#include <random>
#include <algorithm>
#include <string>

#include <RWS.hpp>
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
  for(auto &edge_to : graph.edges()) {
    int j = 0;
    bool last_i = (i == graph.edges().size() - 1);
    auto &from = i;
    for(auto &end : edge_to) {
      auto to = end.first;
      auto gen = end.second;
      bool last_j = (j == edge_to.size() - 1);
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

int main(int argc, char *argv[]) {
  seed_rng();
  std::string fname = (argc > 1) ? argv[1] : "cg_system.txt";
  auto ss = StringSystem::from_file(fname);
  ss.print();

  int max_rules = find_max_rules_optimal(ss, 5.);
  libsemigroups::RWS rws;
  rws_run_knuth_bendix(ss, rws, max_rules);
  printf("rws: completed\n");

  // memoize a part of cayley graph
  for(auto graph_size : {600, 1200, 1800, 3000}) {
    auto cg = CayleyGraph(ss, rws);
    cg.traverse(graph_size, 120.);
    printf("nodes %lu, edges %lu\n", cg.graph.size(), cg.graph.no_edges());
    if(double(cg.graph.size()) * 1.15 > double(cg.graph.no_edges())) {
      continue;
    }
    std::string elems = fname, edges = fname;
    replaceAll(elems, "cg_system", "cg_elements");
    replaceAll(edges, "cg_system", "cg_edges");
    write_elements(elems, cg);
    write_edges(edges, cg);
    std::string adj = fname;
    replaceAll(adj, "cg_system", "cg_adjbitmap");
    cg.graph.to_file_adj(adj.c_str());
    break;
  }
}
