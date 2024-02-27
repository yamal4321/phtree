#include <benchmark/benchmark.h>
#include "ph_tree.h"
#include <vector>
#include <iostream>
#include <chrono>
using u64 = std::uint64_t;

template <u64 D, u64 H> std::vector<typename PHTree<2*D, H>::bstr> gen(int n) {
  using ph = PHTree<2*D, H>;
   auto set = [](u64 *a, double k) {
    std::fill(a, a+(H+63)/64, 0);
    for(auto i=0; i!=int(H*k); i++) a[i/64] |= 1ull << i%64;
  };
  using rect=typename PHTree<2*D, H>::point;
  auto get_rect = [&]() {
    rect r;
    for(auto i=0; i!=D; i++) {
      int norm = 100000;
      int v1 = rand()%norm;
      int v2 = v1 + rand()%(norm-v1+1);
      double p1 = double(v1) / norm;
      double p2 = double(v2) / norm;
      set(r[i], p1);
      set(r[i+D], p2);
    }
    return r;
  };
  std::vector<typename ph::bstr> rects;
  ph tr;
  for(auto i=0; i!=n; i++) rects.push_back(tr.encode(get_rect()));
  return rects;
}

template <u64 D, u64 H, u64 N>
struct Fixture: public benchmark::Fixture {
  using ph = PHTree<2*D, H>;
  using bstr = typename ph::bstr;
  using point = typename ph::point;

  int n=N;
  std::vector<bstr> rects;
  ph t;
  bool is_set=false;

  void SetUp(::benchmark::State& state) {
    if(!is_set) {
      rects = gen<D, H>(n);
      for(auto pt: rects) t.insert(pt);
      is_set=true;
    }
  }
};

BENCHMARK_TEMPLATE_DEFINE_F(Fixture, rect_intersect, ${D}, ${H}, ${N})(benchmark::State& state) {
  constexpr u64 D=${D}, H=${H};
  int i = 0;
  for (auto _ : state) {
      auto rect = gen<D, H>(1)[0];
      auto start = std::chrono::high_resolution_clock::now();
      auto it = t.intersectIterator(rect, false);
      int found=0; for(; !it.end(); it++, found++) { }
      std::cout << found << std::endl;
      i = (i + 1)%int(n);
      auto end = std::chrono::high_resolution_clock::now();
      auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
      state.SetIterationTime(found? elapsed_seconds.count()/double(found): elapsed_seconds.count());
      benchmark::DoNotOptimize(it);
  }
}

BENCHMARK_REGISTER_F(Fixture, rect_intersect)->UseManualTime();
BENCHMARK_MAIN();
