#ifndef RWS_HPP
#define RWS_HPP


#include <chrono>

#include <libsemigroups/src/semigroups.h>
#include <libsemigroups/src/rws.h>
#include <libsemigroups/src/rwse.h>
#include <libsemigroups/src/cong.h>

#include <StringSystem.hpp>


auto now() { return std::chrono::system_clock::now(); }
template <typename T> double diff(T &a, T &b) { return 1e-9 * std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count(); }


bool rws_run_knuth_bendix(StringSystem &ss, libsemigroups::RWS &rws, int max_rules) {
  rws.set_report(false);
  ss.iterate_relators([&](const auto &rel) mutable noexcept -> bool {
    auto lhs = ss.to_str(rel);
    rws.add_rule(lhs.c_str(), "");
    return true;
  });
  rws.set_max_rules(max_rules);
  rws.knuth_bendix();
  return rws.confluent();
}

int find_max_rules_optimal(StringSystem &ss, double max_dur) {
  int max_rules = 100;
  volatile double dur = 0.;
  do {
    max_rules += std::max<int>(max_rules / 8, 100);
    const auto start = now();
    {
      libsemigroups::RWS rws;
      bool confl = rws_run_knuth_bendix(ss, rws, max_rules);
      if(confl)return -1;
    }
    const auto stop = now();
    dur = diff(start, stop);
    printf("rws: max_rules=%d dur=%.2f\n", max_rules, dur);
  } while(dur < max_dur);
  printf("rws: optimal max_rules=%d dur=%.2f\n", max_rules, dur);
  return max_rules;
}


#endif /* end of include guard: RWS_HPP */
