#ifndef GRAPH_HPP
#define GRAPH_HPP


#include <map>
#include <vector>
#include <string>
#include <set>

#include <libsemigroups/src/semigroups.h>
#include <libsemigroups/src/rws.h>
#include <libsemigroups/src/rwse.h>
#include <libsemigroups/src/cong.h>

#include <StringSystem.hpp>


auto now() { return std::chrono::system_clock::now(); }
template <typename T> double diff(T &a, T &b) { return 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count(); }


template <typename T>
struct Graph {
  std::vector<T> vertices_;
  std::map<int, std::map<int, StringSystem::gen_t>> edges_;

  Graph():
    vertices_(), edges_()
  {}

  bool empty() const noexcept {
    return size() == 0;
  }

  size_t size() const noexcept {
    return vertices_.size();
  }

  const auto &vertices() noexcept {
    return vertices_;
  }

  const auto &edges() noexcept {
    return edges_;
  }

  auto &operator[](int key) noexcept {
    return edges_[key];
  }

  void add_vertex(T val=T()) noexcept {
    vertices_.push_back(val);
  }

  void add_edge(int i, int j, StringSystem::gen_t g) noexcept {
    edges_[i][j] = g;
  }
};


struct CayleyGraph {
  std::set<int> visited;
  Graph<std::string> *graph_ptr;
  Graph<std::string> &graph;
  StringSystem &ss;
  libsemigroups::RWS &rws;

  using gen_t = StringSystem::gen_t;
  using word_t = StringSystem::word_t;

  CayleyGraph(StringSystem &ss, libsemigroups::RWS &rws) noexcept:
    graph_ptr(new Graph<std::string>()), graph(*graph_ptr), ss(ss), rws(rws)
  {
    graph.add_vertex();
  }

  void visit(size_t ind) noexcept {
    if(visited.count(ind))return;

    assert(ind < graph.size());
    const std::string g = graph.vertices()[ind];
    for(int i = 0; i < ss.size(); ++i) {
      auto gen = ss[i];
      const std::string n = g + ss.to_str(gen);

      int h_ind;
      for(h_ind = 0; h_ind < graph.size(); ++h_ind) {
        const std::string h = graph.vertices()[h_ind];
        if(rws.test_equals(n, h))break;
      }

      if(h_ind == graph.size()) {
        if((graph.size() + 1) % 50 == 0) {
          printf("[%d] new vertex %s\n", h_ind, n.c_str());
        }
        graph.add_vertex(n);
      }

      graph[ind][h_ind] = gen;
    }

    visited.insert(ind);
  }

  void traverse(int limit, double max_dur, int min_depth=4) {
    int graph_ind = 0;
    auto start = now();
    while(graph.size() < limit) {
      auto stop = now();
      visit(graph_ind);
      ++graph_ind;
      auto dur = diff(start, stop);

      // minimum depth not reached, top priority
      if(graph.vertices()[graph.size() - 1].length() < min_depth)continue;

      // max graph size reached, retreat
      if(graph.size() > limit)break;

      // took too long to get here, retreat
      if(dur > max_dur)break;
    }
  }

  size_t size() const { return graph.size(); }
  ~CayleyGraph() { delete graph_ptr; }
};

struct CGTransition {
  CayleyGraph &cg;
  size_t ind;
  size_t to;

  CGTransition(CayleyGraph &cg, size_t ind, size_t to) noexcept:
    cg(cg), ind(ind), to(to)
  {}

  std::string trans() {
    cg.visit(ind);
    return cg.ss.to_str(cg.graph[ind][to]);
  }
};


template <typename F>
struct CGTransitionView {
  CayleyGraph &cg;
  F &&func;
  size_t size_;

  CGTransitionView(CayleyGraph &cg, F &&func, size_t size):
    cg(cg), func(std::forward<F>(func)), size_(size)
  {}

  decltype(auto) operator[](size_t ind) {
    return func(ind);
  }

  size_t size() const {
    return size_;
  }
};

template <typename F>
decltype(auto) make_cg_tview(CayleyGraph &cg, size_t size, F &&func) {
  return CGTransitionView<F>(cg, std::forward<F>(func), size);
}


struct CGVisitor {
  CayleyGraph &cg;
  size_t cur_node = 0;

  CGVisitor(CayleyGraph &cg):
    cg(cg)
  {}

  decltype(auto) get_neighbors() {
    cg.visit(cur_node);
    auto &outs = cg.graph[cur_node];
    return make_cg_tview(cg, outs.size(), [=](auto to) mutable noexcept -> CGTransition {
      return CGTransition(cg, cur_node, to);
    });
  }
};


#endif /* end of include guard: GRAPH_HPP */
