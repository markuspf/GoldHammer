#ifndef STRINGSYSTEM_HPP
#define STRINGSYSTEM_HPP


#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cassert>

#include <iostream>
#include <type_traits>
#include <random>
#include <algorithm>
#include <string>
#include <vector>
#include <set>


#include <fcntl.h>
#include <unistd.h>

void seed_rng() {
  int fd=open("/dev/urandom", O_RDONLY);
  if(fd==-1) abort();
  unsigned seed,pos = 0;
  while(pos<sizeof(seed)){int a=read(fd,(char*)&seed+pos,sizeof(seed)-pos);if(a<=0)abort();pos += a;}
  srand(seed);
  close(fd);
}


template <typename... Ts> void unroll(Ts...){}

struct StringSystem {
  using gen_t = int;
  using word_t = std::vector<gen_t>;

  int no_generators = 0;
  std::vector<word_t> relators;

  StringSystem(size_t n=0) noexcept:
    relators()
  {
    for(int i = 0; i < n; ++i) {
      add_generator();
    }
  }

  gen_t add_generator() noexcept {
    gen_t g = size();
    auto gen = g;
    auto inv = inverse(gen);
    relators.push_back({gen, inv});
    relators.push_back({inv, gen});
    ++no_generators;
    return g;
  }

  // Aa
  bool is_simple_relator(const word_t &w) noexcept { return w.size() == 2 && w[0] == inverse(w[1]); }
  bool is_in_word(const word_t &w, gen_t g) noexcept { return std::find(std::begin(w), std::end(w), g) != std::end(w); }

  void remove_simple_relators(gen_t g) noexcept {
    if(g < 0)g = inverse(g);
    gen_t G = inverse(g);
    for(int i = 0; i < relators.size();) {
      auto &r = relators[i];
      if(is_simple_relator(r) && (r[0] == g || r[1] == g)) {
        relators.erase(std::begin(relators) + i);
      } else {
        ++i;
      }
    }
  }

  void replace_gen_with_word(word_t &w, gen_t gen, const word_t &word) {
    replace_word(w, {gen}, word);
    replace_word(w, {inverse(gen)}, inverse(word));
  }

  void simplify_relators() noexcept {
    StringSystem ss = *this;
    for(auto &r : ss.relators) {
      if(is_simple_relator(r))continue;
      simplify_word_simple(r);
    }
    relators = ss.relators;
  }

  void sort_relators() {
    decltype(relators) new_relators;
    for(auto &r : relators)if(is_simple_relator(r))new_relators.push_back(r);
    for(auto &r : relators)if(!is_simple_relator(r))new_relators.push_back(r);
    relators = new_relators;
  }

  void remove_generator(gen_t g, const word_t &replacement) noexcept {
    /* std::cout << "REMOVE GENERATOR" << std::endl; */
    /* print(); */
    if(g < 0)g = inverse(g);
    auto G = inverse(g);
    remove_simple_relators(operator[](size() - 1));
    for(auto &r : relators) {
      if(is_simple_relator(r))continue;
      replace_gen_with_word(r, g, replacement);
      for(auto &x : r) {
        if(x > g) {
          --x;
        } else if(x < G) {
          auto &X = x;
          X = inverse(inverse(X) - 1);
        }
      }
    }
    simplify_relators();
    --no_generators;
    sort_relators();
  }

  constexpr size_t size() const noexcept {
    return no_generators;
  }

  void set_order(const word_t &word, size_t p) noexcept {
    auto rel = word_t();
    for(int i = 0; i < p; ++i) {
      for(auto &e : word) {
        rel.push_back(e);
      }
    }
    relators.push_back(rel);
  }

  void add_relator(const word_t &w) {
    set_order(w, 1);
  }

  template <typename F>
  void iterate_relators(F &&func) {
    for(int i = 0; i < relators.size(); ++i) {
      if(!func(relators[i]))break;
    }
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

  word_t inverse(const word_t &w) noexcept {
    word_t inv = w;
    for(int i = w.size() - 1; i >= 0; --i) {
      inv[w.size() - i - 1] = inverse(w[i]);
    }
    return inv;
  }

/*   // relators += [word = 1] */
/*   void fp_add_relator(const word_t &&word) noexcept { */
/*     // word has to be 1 in G */
/*     relators.push_back(word); */
/*   } */

/*   // relators += [r[i] * r[j] = 1] */
/*   // del relators[i] */
/*   void fp_replace_by_product(size_t i, size_t j) noexcept { */
/*     word_t rel = multiply(relators[i], relators[j]); */
/*     relators.erase(std::begin(relators) + i); */
/*     relators.push_back(rel); */
/*   } */

/*   // word has to be 1 in G */
/*   // generators += [n] */
/*   // relators += [g_n^{-1} * w = 1] */
/*   void fp_add_generator(const word_t &w) noexcept { */
/*     gen_t g = add_generator(); */
/*     word_t inv = {inverse(g)}; */
/*     relators.push_back(multiply(inv, w)); */
/*   } */

  // word can be anything in G
  void fp_add_shortcut(const word_t &w) noexcept {
    gen_t g = add_generator();
    word_t inv = {inverse(g)};
    relators.push_back(multiply(inv, w));
    sort_relators();
  }

  void fp_uglify_add_generator() noexcept {
    if(size() <= 1)return;
    auto g = add_generator();
    // pick a generator
    gen_t t;
    do { t = random_gen(); } while(t == g);

    StringSystem ss = *this;
    bool check_failed = false;
    do {
      if(check_failed) {
        *this = ss;
      }
      // make a word
      word_t w;
      do {
        w = random_word(4);
        replace_word(w, {t}, {});
        replace_word(w, {inverse(t)}, {});
        simplify_word(w);
      } while(w.size() < 3 || std::find(std::begin(w), std::end(w), g) == std::end(w));
      // set the equation
      /* auto st = SSTransformator(*this); */
      /* SSTransformator::SSEquation eqn(st); */
      /* eqn.lhs = {t}; */
      /* eqn.rhs = w; */
      /* std::cout << eqn.to_str() << std::endl; */
      remove_generator(t, w);

      check_failed = false;
      iterate_relators([&](auto &rel) mutable noexcept -> bool {
        if(rel.empty() || rel.size() > 150) {
          check_failed = true;
          return false;
        }
        return !check_failed;
      });
    } while(check_failed);
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
    while(s[s.length() - 1] == ' ')s.pop_back();
    return s;
  }

  int str_word_len(const std::string &s_w) {
    if(size() > 26) {
      return std::count(s_w.begin(), s_w.end(), ' ') + 1;
    }
    return s_w.length();
  }

  void print() {
    std::cout << "GENERATORS" << std::endl;
    for(int i = 0; i < size(); ++i) {
      puts(to_str(i).c_str());
    }
    puts("");
    std::cout << "RELATORS" << std::endl;
    for(auto &r : relators) {
      puts(to_str(r).c_str());
    }
  }

  typedef enum {
    SHORTCUT,
    REMOVE,
    /* GENERATE, */
    /* RELATE, */
    /* REPLACE, */
    NO_OPTIONS
  } uglify;

  void replace_word(word_t &w, const word_t &from, const word_t &to) { replace_word(w, std::forward<const word_t>(from), std::forward<const word_t>(to)); }
  void replace_word(word_t &w, const word_t &&from, const word_t &&to) {
    if(w.size() < from.size())return;
    auto result = word_t{};
    int i = 0;
    while(i <= w.size() - from.size()) {
      bool replace=true;
      for(int j = 0; j < from.size() && replace; ++j)if(w[i + j] != from[j])replace=false;
      if(replace){
        word_append(result,to);
        i += from.size();
      }else{
        word_append(result,w[i]);
        ++i;
      }
    }
    /* printf("replace word [%s]: [%s]/[%s] -> [%s]\n", */
    /*   to_str(w).c_str(), */
    /*   to_str(from).c_str(), */
    /*   to_str(to).c_str(), */
    /*   to_str(result).c_str()); */
    w = result;
  }

  void simplify_with_relator(word_t &w, const word_t &rel) {
    if(w.size() < rel.size())return;
    word_t v = w;
    for(int i = 0; i <= w.size() - rel.size(); ++i) {
      for(int j = 0; j < rel.size(); ++j) {
        if(w[i + j] != rel[j]) {
          goto endloop;
        }
      }
      for(int k = 0; k < rel.size(); ++k) {
        w.erase(std::begin(w) + i);
      }
      break;
      endloop:;
    }
    /* printf("simplification [%s]: [%s] -> [%s]\n", to_str(rel).c_str(), to_str(v).c_str(), to_str(w).c_str()); */
  }

  void simplify_word_simple(word_t &w) {
    word_t v = w;
    int len = w.size();
    do {
      word_t u = w;
      len = w.size();
      iterate_relators([&](auto &rel) mutable noexcept -> bool {
        if(is_simple_relator(rel)) {
          simplify_with_relator(w, rel);
        }
        return true;
      });
    } while(w.size() < len);
  }

  void simplify_word(word_t &w) {
    word_t v = w;
    int len = w.size();
    do {
      word_t u = w;
      len = w.size();
      iterate_relators([&](auto &rel) mutable noexcept -> bool {
        simplify_with_relator(w, rel);
        return true;
      });
      /* printf("simplify step [%s] -> [%s]\n", to_str(u).c_str(), to_str(w).c_str()); */
    } while(w.size() < len);
    /* printf("simplify [%s] -> [%s]\n", to_str(v).c_str(), to_str(w).c_str()); */
  }

  void word_insert(word_t &w, gen_t g) { w.insert(std::begin(w), g); }
  void word_insert(word_t &w, const word_t &a) { for(gen_t g : a)word_insert(w, g); }
  void word_append(word_t &w, gen_t g) { w.push_back(g); }
  void word_append(word_t &w, const word_t &a) { for(gen_t g : a)word_append(w, g); }
  void word_erase(word_t &w, size_t ind) { w.erase(std::begin(w) + ind); }
  gen_t random_gen() { return (*this)[rand() % size()]; }

  std::set<word_t> used_words;
  word_t random_word(int len=-1) {
    if(len == -1)len = rand() % 15;
    word_t w;
    do {
        do {
          for(int i = 0; i < len; ++i) {
            gen_t g = (*this)[rand() % size()];
            if(rand() & 1) {
              g = inverse(g);
            }
            w.push_back(g);
            simplify_word(w);
          }
        } while(w.size() < len);
    } while(used_words.count(w));
    used_words.insert(w);
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
      case SHORTCUT:
        fp_add_shortcut(random_word(5));
      break;
      case REMOVE:
        fp_uglify_add_generator();
      break;
      /* case GENERATE: */
        /* fp_add_generator(random_identity()); */
      /* break; */
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
    iterate_relators([&](auto &rel) mutable noexcept -> bool {
      for(int i = 0; i < rel.size(); ++i) {
        if(i != 0)fprintf(fp, " ");
        fprintf(fp, "%d", rel[i]);
      }
      fprintf(fp, "\n");
      return true;
    });
    fclose(fp);
  }

  static decltype(auto) from_file(std::string fname) {
    StringSystem ss(0);

    FILE *fp = fopen(fname.c_str(), "r");
    if(fp == nullptr)abort();
    int len;
    fscanf(fp, "%d\n", &len);
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


struct SSTransformator {
  using word_t = StringSystem::word_t;
  using gen_t = StringSystem::gen_t;

  StringSystem &ss;

  SSTransformator(StringSystem &ss):
    ss(ss)
  {}

  struct SSEquation {
    using word_t = SSTransformator::word_t;
    using gen_t = SSTransformator::gen_t;
    SSTransformator &st;
    word_t
      lhs = {},
      rhs = {};

    SSEquation(SSTransformator &st):
      st(st)
    {}

    std::string to_str() {
      auto &ss = st.ss;
      std::string s_lhs = lhs.empty() ? "1" : ss.to_str(lhs);
      std::string s_rhs = rhs.empty() ? "1" : ss.to_str(rhs);
      std::string s = s_lhs + " == " + s_rhs;
      return s;
    }

    void multiply_left(gen_t g) {
      auto &ss = st.ss;
      ss.word_insert(lhs, g);
      ss.simplify_word(lhs);
      ss.word_insert(rhs, g);
      ss.simplify_word(rhs);
    }

    void multiply_right(gen_t g) {
      auto &ss = st.ss;
      ss.word_append(lhs, g);
      ss.simplify_word(lhs);
      ss.word_append(rhs, g);
      ss.simplify_word(rhs);
    }

    void simplify() {
      auto &ss = st.ss;
      int len = 0;
      while(1) {
        while(!(lhs.empty() || rhs.empty()) && lhs[0] == rhs[0]) {
          ss.word_erase(lhs, 0);
          ss.word_erase(rhs, 0);
        }
        while(!(lhs.empty() || rhs.empty()) && lhs[lhs.size() - 1] == rhs[rhs.size() - 1]) {
          lhs.pop_back();
          rhs.pop_back();
        }
        ss.simplify_word(lhs);
        ss.simplify_word(rhs);
        int new_len = lhs.size() + rhs.size();
        if(len == new_len)break;
        len = new_len;
      }
    }

    void cancel_lhs_left(int n=1) {
      for(int i = 0; i < n; ++i) {
        if(lhs.empty())return;
        auto &ss = st.ss;
        auto g = lhs[0];
        auto G = ss.inverse(g);
        ss.word_insert(rhs, G);
        ss.word_erase(lhs, 0);
      }
    }

    void cancel_lhs_right(int n=1) {
      for(int i = 0; i < n; ++i) {
        if(lhs.empty())return;
        auto &ss = st.ss;
        auto g = lhs[lhs.size() - 1];
        auto G = ss.inverse(g);
        ss.word_append(rhs, G);
        lhs.pop_back();
      }
    }

    void cancel_rhs_left(int n=1) {
      for(int i = 0; i < n; ++i) {
        if(rhs.empty())return;
        auto &ss = st.ss;
        auto g = rhs[0];
        auto G = ss.inverse(g);
        ss.word_insert(lhs, G);
        ss.word_erase(rhs, 0);
      }
    }

    void cancel_rhs_right(int n=1) {
      for(int i = 0; i < n; ++i) {
        if(rhs.empty())return;
        auto &ss = st.ss;
        auto g = rhs[rhs.size() - 1];
        auto G = ss.inverse(g);
        ss.word_append(lhs, G);
        rhs.pop_back();
      }
    }

    void cancel_lhs() {
      while(!lhs.empty()) {
        simplify();
        cancel_lhs_left();
      }
    }

    void cancel_rhs() {
      while(!rhs.empty()) {
        simplify();
        cancel_rhs_left();
      }
    }
  };

  decltype(auto) make_eqns() {
    std::vector<SSEquation> equations;
    ss.iterate_relators([&](auto &rel) mutable noexcept -> bool {
      SSEquation eq(*this);
      eq.lhs = rel;
      eq.rhs = word_t{};
      equations.push_back(eq);
      return true;
    });
    return equations;
  }

  void print_eqns(std::vector<SSEquation> &eqns) {
    std::cout << "Equations:" << std::endl;
    for(auto &eq : eqns) {
      std::cout << eq.to_str() << std::endl;
    }
  }
};


#endif /* end of include guard: STRINGSYSTEM_HPP */
