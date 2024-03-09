#include <benchmark/benchmark.h>
#include <vector>
#include <iostream>
#include "ph_tree_parallel.h"

using u64 = std::uint64_t;

template <u64 D, u64 H> std::vector<typename PHTree<D, H>::bstr> gen(int n) {
  using ph = PHTree<D, H>;
  using bstr = typename ph::bstr;
  using point = typename ph::point;

  std::vector<bstr> ret;

  ph tr;
  for(auto i=0; i!=n; i++) {
    point p;
    for(auto j=0; j!=D; j++) for(auto k=0; k!=ph::POINT_K; k++) p[j][k] = (u64(rand()) << 32) | u64(rand());
    ret.push_back(tr.encode(p));
  }

  return ret;
}

template <u64 D, u64 H, u64 N>
struct Fixture: public benchmark::Fixture {
  using ph = PHTree<D, H>;
  using bstr = typename ph::bstr;
  using point = typename ph::point;

  int n=N;
  std::vector<bstr> to_insert;
  std::vector<void*> to_insertp;
  bool is_set=false;

  void SetUp(::benchmark::State& state) {
    if(!is_set) {
      to_insert = gen<D, H>(n);
      to_insertp = std::vector<void*>(n, nullptr);
      is_set=true;
    }
  }
};

BENCHMARK_TEMPLATE_DEFINE_F(Fixture, insert, ${D}, ${H}, ${N})(benchmark::State& state) {
  for (auto _ : state) {
    {
      state.PauseTiming();
      ph t;
      state.ResumeTiming();
      t.insert(to_insert, to_insertp, ${C}); 
      state.PauseTiming();
      benchmark::DoNotOptimize(t);
    }
    state.ResumeTiming();
  }
}

BENCHMARK_REGISTER_F(Fixture, insert);
BENCHMARK_MAIN();
