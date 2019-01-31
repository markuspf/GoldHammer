#ifndef STRINGSYSTEM_HPP
#define STRINGSYSTEM_HPP


#include <iostream>
#include <type_traits>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <random>
#include <algorithm>
#include <string>
#include <vector>


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
    auto rel = word_t();
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

  // word can be anything in G
  void fp_add_shortcut(const word_t &w) noexcept {
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
    /* RELATE, */
    /* REPLACE, */
    NO_OPTIONS
  } uglify;

  void simplify_with_relator(word_t &w, const word_t &rel) {
    if(w.size() < rel.size())return;
    for(int i = 0; i < w.size() - rel.size(); ++i) {
      for(int j = 0; j < rel.size(); ++j) {
        if(w[i + j] != rel[j]) {
          goto endloop;
        }
      }
      for(int k = 0; k < rel.size(); ++k) {
        w.erase(std::begin(w) + i);
      }
      return;
      endloop:;
    }
  }

  void simplify_word(word_t &w) {
    int len = w.size();
    do {
      for(const auto &r : relators) {
        simplify_with_relator(w, r);
      }
      len = w.size();
    } while(w.size() < len);
  }

  word_t random_word(int len=-1) {
    if(len == -1)len = rand() % 15;
    word_t w;
    do {
      for(int i = 0; i < len; ++i) {
        gen_t g = (*this)[rand() % size()];
        if(rand()) {
          g = inverse(g);
        }
        w.push_back(g);
        simplify_word(w);
      }
    } while(w.size() < len);
    return w;
  }

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
    /* if(choice == GENERATE && size() == 26) { */
    /*   choice = (rand() & 1) + GENERATE + 1; */
    /* } */
    switch(choice) {
      case GENERATE:
        /* fp_add_generator(random_identity()); */
        fp_add_shortcut(random_word());
      break;
      /* case RELATE: */
      /*   fp_add_relator(random_identity()); */
      /* break; */
      /* case REPLACE: */
      /*   int i = rand() % size(); */
      /*   int j = -1; */
      /*   do { */
      /*     j = rand() % relators.size(); */
      /*   } while(j == i); */
      /*   fp_replace_by_product(i, j); */
      /* break; */
    }
    fp_uglify(no_iter - 1);
  }

  void to_file(std::string fname) {
    FILE *fp = fopen(fname.c_str(), "w");
    if(fp == nullptr)abort();
    fprintf(fp, "%lu\n", size());
    for(auto &r: relators) {
      for(int i = 0; i < r.size(); ++i) {
        if(i != 0)fprintf(fp, " ");
        fprintf(fp, "%d", r[i]);
      }
      fprintf(fp, "\n");
    }
    fclose(fp);
  }

  static decltype(auto) from_file(std::string fname) {
    StringSystem ss(0);

    FILE *fp = fopen(fname.c_str(), "r");
    if(fp == nullptr)abort();
    int len;
    fscanf(fp, "%d", &len);
    ss.no_generators = len;
    word_t w;
    while(1) {
      char c;
      if((c = fgetc(fp)) == '\n') {
        ss.relators.push_back(w);
        if((c = fgetc(fp)) == EOF) {
          break;
        } else {
          ungetc(c, fp);
        }
        w = word_t();
      } else {
        ungetc(c, fp);
      }
      gen_t g;
      fscanf(fp, " %d", &g);
      w.push_back(g);
    }
    fclose(fp);
    return ss;
  }
};


#endif /* end of include guard: STRINGSYSTEM_HPP */
