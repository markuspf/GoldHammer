#include <iostream>
#include <type_traits>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <random>
#include <algorithm>
#include <map>
#include <queue>
#include <fcntl.h>
#include <unistd.h>

#include <libsemigroups/src/semigroups.h>
#include <libsemigroups/src/rws.h>
#include <libsemigroups/src/rwse.h>
#include <libsemigroups/src/cong.h>


#include <StringSystem.hpp>


template <typename T>
struct Graph {
  std::vector<T> vertices_;
  std::map<int, std::set<int>> edges_;

  Graph():
    vertices_(), edges_()
  {}

  bool empty() const noexcept {
    return size() == 0;
  }

  size_t size() const noexcept {
    return vertices_.size();
  }

  const auto &vertices() const noexcept {
    return vertices_;
  }

  const auto &edges() const noexcept {
    return edges_;
  }

  void add_vertex(T val) noexcept {
    vertices_.push_back(val);
  }

  void add_edge(int i, int j) noexcept {
    edges_[i].insert(j);
  }
};


auto *make_cayley_graph(StringSystem &system, libsemigroups::RWS &rws, int limit=300) {
  auto *graph_ptr = new Graph<std::string>();
  auto &graph = *graph_ptr;
  graph.add_vertex("");
  std::set<int> seen;
  using word_t = StringSystem::word_t;
  size_t graph_ind = 0;

  std::vector<int> generators(system.size());
  for(int i = 0; i < generators.size(); ++i) {
      generators[i] = i;
  }
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(std::begin(generators), std::end(generators), g);

  do {
    // take new element, g
    int left = graph.size();
    while(1) {
      bool change = false;
      if(graph_ind >= graph.size()) {
        graph_ind = 0;
        --left;
        change = true;
      }
      if(!left) {
        break;
      }
      if(seen.find(graph_ind) != std::end(seen)) {
        ++graph_ind;
        --left;
        change = true;
      }
      if(!change || !left) {
        if(left) {
          seen.insert(graph_ind);
        }
        break;
      }
    }
    if(!left) {
      break;
    }

    std::string g = graph.vertices()[graph_ind];
    std::cout << "take '" << g << "'" << std::endl;

    // try different outcomes from it
    for(auto i : generators) {
      auto n = g + system.to_str(system[i]);
      std::cout << "\tconsider '" << n << "'" << std::endl;
      // check if the new word is already cached
      int j = 0;
      std::string h_save;
      for(auto h : graph.vertices()) {
        if(rws.test_equals(n.c_str(), h.c_str())) {
          h_save = h;
          std::cout << "\t\texists: '" << n << "' == '" << h << "'" << std::endl;
          break;
        }
        ++j;
      }

      // is a new element
      auto g_ind = graph_ind;
      auto h_ind = j;
      if(h_ind == graph.size()) {
        std::cout << "\t-> new vertex '" << n << "'" << std::endl;
        graph.add_vertex(n);
      } else {
        auto h = h_save;
      }
      graph.add_edge(g_ind, h_ind);
      std::cout << "\t\tconnect: " << g_ind << " to " << h_ind << std::endl;
    }

    ++graph_ind;
  } while(--limit && seen.size() < graph.size());
  return graph_ptr;
}


void write_elements(std::string filename, Graph<std::string> &graph) {
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

void write_edges(std::string filename, Graph<std::string> graph) {
  FILE *fp = fopen(filename.c_str(), "w");
  /* FILE *fp = stdout; */
  fprintf(fp, "[");
  int line = 1;
  int i = 0;
  for(auto &edge : graph.edges()) {
    int j = 0;
    bool last_i = (i == graph.edges().size() - 1);
    auto &from = edge.first;
    for(auto &to : edge.second) {
      bool last_j = (j == edge.second.size() - 1);
      std::string e = " [";
      e += std::to_string(from + 1);
      e += ", ";
      e += std::to_string(to + 1);
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


void seed_rng() {
  int fd=open("/dev/urandom", O_RDONLY);
  if(fd==-1) abort();
  unsigned seed,pos = 0;
  while(pos<sizeof(seed)){int a=read(fd,(char*)&seed+pos,sizeof(seed)-pos);if(a<=0)abort();pos += a;}
  srand(seed);
  close(fd);
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
  auto t = StringSystem::from_file(fname);
  t.print();
  libsemigroups::RWS rws;
  rws.set_report(true);
  for(const auto &rel : t.relators) {
    auto lhs = t.to_str(rel);
    rws.add_rule(lhs.c_str(), "");
    /* std::cout << "ADD RULE " << lhs << std::endl; */
  }
  rws.set_max_rules(800);
  rws.knuth_bendix();
  auto *g = make_cayley_graph(t, rws, 300);
  fflush(stdout);

  std::string elems = fname, edges = fname;
	replaceAll(elems, "cg_system", "cg_elements");
	replaceAll(edges, "cg_system", "cg_edges");
  write_elements(elems, *g);
  write_edges(edges, *g);
  delete g;
}
