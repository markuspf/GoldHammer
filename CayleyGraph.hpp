#ifndef GRAPH_HPP
#define GRAPH_HPP


#include <map>
#include <vector>

#include <StringSystem.hpp>


template <typename T>
struct Graph {
  std::vector<T> vertices_;
  std::map<int, std::set<std::pair<int, StringSystem::gen_t>>> edges_;

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

  void add_edge(int i, int j, StringSystem::gen_t g) noexcept {
    edges_[i].insert(std::pair<decltype(j), decltype(g)>(j, g));
  }
};


#endif /* end of include guard: GRAPH_HPP */
