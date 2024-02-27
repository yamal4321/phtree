#include <benchmark/benchmark.h>
#include "ph_tree.h"
#include <vector>
#include <iostream>

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

template <u64 D, u64 H>
struct Fixture: public benchmark::Fixture {
  using ph = PHTree<D, H>;
  using bstr = typename ph::bstr;
  using point = typename ph::point;

  int n=1e6;
  std::vector<bstr> pts;
  std::vector<point> bpts;
  bool is_set=false;

  void SetUp(::benchmark::State& state) {
    ph tr;
    if(!is_set) {
      pts = gen<D, H>(n);
      for(auto pt: pts) bpts.push_back(tr.decode(pt));
      is_set=true;
    }
  }
};

BENCHMARK_TEMPLATE_DEFINE_F(Fixture, decode, ${D}, ${H})(benchmark::State& state) {
  PHTree<${D}, ${H}> tr;
  int i = 0;
  for (auto _ : state) {
    auto pt=tr.decode(pts[i]); i = (i + 1)%int(n);  
    benchmark::DoNotOptimize(pt);
  }
}

BENCHMARK_TEMPLATE_DEFINE_F(Fixture, encode, ${D}, ${H})(benchmark::State& state) {
  PHTree<${D}, ${H}> tr;
  int i = 0;
  for (auto _ : state) {
    auto pt=tr.encode(bpts[i]); i = (i + 1)%int(n);  
    benchmark::DoNotOptimize(pt);
  }
}

BENCHMARK_REGISTER_F(Fixture, decode);
BENCHMARK_MAIN();
