#include <iostream>
#include <type_traits>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <random>
#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <queue>

#include <libsemigroups/src/semigroups.h>
#include <libsemigroups/src/rws.h>
#include <libsemigroups/src/rwse.h>
#include <libsemigroups/src/cong.h>


template <typename... Ts> void unroll(Ts...){}

struct StringSystem {
  using gen_t = int;
  using word_t = std::vector<gen_t>;

  int no_generators = 0;
  std::vector<word_t> relators;

  StringSystem(size_t n) noexcept:
    relators()
  {
    for(int i = 0; i < n; ++i) {
      add_generator();
    }
  }

  gen_t add_generator() {
    gen_t g = size();
    auto gen = g;
    auto inv = inverse(gen);
    relators.push_back({gen, inv});
    relators.push_back({inv, gen});
    ++no_generators;
    return g;
  }

  constexpr size_t size() const noexcept {
    return no_generators;
  }

  void set_order(const word_t &&word, size_t p) noexcept {
    auto rel = std::remove_reference_t<decltype(word)>();
    for(int i = 0; i < p; ++i) {
      for(auto &e : word) {
        rel.push_back(e);
      }
    }
    relators.push_back(rel);
  }

  gen_t operator[](size_t i) const noexcept {
    return i;
  }

  static word_t multiply(const word_t &a, const word_t &b) noexcept {
    word_t res;
    for(const auto &g : a) {
      res.push_back(g);
    }
    for(const auto &g : b) {
      res.push_back(g);
    }
    return res;
  }

  static gen_t inverse(const gen_t &g) noexcept {
    return -g - 1;
  }

  // relators += [word = 1]
  void fp_add_relator(const word_t &&word) noexcept {
    // word has to be 1 in G
    relators.push_back(word);
  }

  // relators += [r[i] * r[j] = 1]
  // del relators[i]
  void fp_replace_by_product(size_t i, size_t j) noexcept {
    word_t rel = multiply(relators[i], relators[j]);
    relators.erase(std::begin(relators) + i);
    relators.push_back(rel);
  }

  // word has to be 1 in G
  // generators += [n]
  // relators += [g_n^{-1} * w = 1]
  void fp_add_generator(const word_t &w) noexcept {
    gen_t g = add_generator();
    word_t inv = {inverse(g)};
    relators.push_back(multiply(inv, w));
  }

  std::string to_str(gen_t gen) const {
    std::string s;
    gen_t pos = gen;
    if(gen < 0) {
      pos = inverse(gen);
    }
    if(size() > 26) {
      s = std::string("g") + std::to_string(pos) + " ";
    } else {
      s = 'a' + pos;
    }
    if(gen < 0) {
      for(auto &c : s) {
        c = toupper(c);
      }
    }
    return s;
  }

  std::string to_str(const word_t &w) const {
    std::string s;
    for(int i = 0; i < w.size(); ++i) {
      s += to_str(w[i]);
      /* if(i != w.size() - 1) { */
      /*   s += "*"; */
      /* } */
    }
    return s;
  }

  void print() {
    std::cout << "GENERATORS" << std::endl;
    for(int i = 0; i < size(); ++i) {
      puts(to_str(i).c_str());
    }
    puts("");
    std::cout << "RELATORS" << std::endl;
    for(auto r : relators) {
      puts(to_str(r).c_str());
    }
  }

  typedef enum {
    GENERATE,
    RELATE,
    REPLACE,
    NO_OPTIONS
  } uglify;

  word_t random_identity() {
    int i = rand() % relators.size();
    int j = -1;
    do {
      j = rand() % relators.size();
    } while(j == i);
    return multiply(relators[i], relators[j]);
  }

  inline void fp_uglify(int no_iter = 100) noexcept {
    if(no_iter == 0)return;
    int choice = rand() % NO_OPTIONS;
    if(choice == GENERATE && size() == 26) {
      choice = (rand() & 1) + GENERATE + 1;
    }
    switch(choice) {
      case GENERATE:
        fp_add_generator(random_identity());
      break;
      case RELATE:
        fp_add_relator(random_identity());
      break;
      case REPLACE:
        int i = rand() % size();
        int j = -1;
        do {
          j = rand() % relators.size();
        } while(j == i);
        fp_replace_by_product(i, j);
      break;
    }
    fp_uglify(no_iter - 1);
  }
};


StringSystem triangle_group(int p, int q, int r) {
  auto t = StringSystem(2);
  StringSystem::gen_t x = t[0], y = t[1];
  t.set_order({x}, p);
  t.set_order({y}, q);
  t.set_order({x, y}, r);
  return t;
}


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


void test_rws(StringSystem &system, libsemigroups::RWS &rws) {
  std::string a = system.to_str(0);
  std::string b = system.to_str(1);
  assert(rws.test_equals(a+a, ""));
  assert(rws.test_equals(b+b+b, ""));
  auto ab = a + b;
  assert(rws.test_equals(ab+ab+ab+ab+ab+ab+ab, ""));
}


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

int main(int argc, char *argv[]) {
  srand(time(nullptr));
  auto t = triangle_group(2, 3, 7);
  /* t.fp_uglify(); */
  t.print();
  libsemigroups::RWS rws;
  rws.set_report(true);
  for(const auto &rel : t.relators) {
    auto lhs = t.to_str(rel);
    rws.add_rule(lhs.c_str(), "");
    std::cout << "ADD RULE " << lhs << std::endl;
  }
  rws.set_max_rules(800);
  rws.knuth_bendix();
  test_rws(t, rws);
  auto *g = make_cayley_graph(t, rws, 300);
  fflush(stdout);
  write_elements("cg_elements.txt", *g);
  write_edges("cg_edges.txt", *g);
  delete g;
}
