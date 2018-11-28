#include <iostream>
#include <string>
#include <type_traits>
#include <cctype>
#include <cstdlib>
#include <ctime>

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

  void fp_uglify(int no_iter = 10) noexcept {
    if(no_iter == 0)return;
    int choice = rand() % NO_OPTIONS;
    if(choice == 0 && size() > 26) {
      fp_uglify(no_iter);
      return;
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
  StringSystem t(2);
  StringSystem::gen_t x = 0, y = 1;
  t.set_order({x}, p);
  t.set_order({y}, q);
  t.set_order({x, y}, r);
  return t;
}


int main(int argc, char *argv[]) {
  srand(time(nullptr));
  auto t = triangle_group(2, 3, 7);
  t.fp_uglify();
  /* t.print(); */
  libsemigroups::RWS rws;
  rws.set_report(true);
  for(const auto &rel : t.relators) {
    auto lhs = t.to_str(rel);
    rws.add_rule(lhs.c_str(), "");
    std::cout << "ADD RULE " << lhs << std::endl;
  }
  rws.knuth_bendix();
}
