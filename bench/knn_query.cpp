#include <benchmark/benchmark.h>
#include "ph_tree.h"
#include <vector>
#include <iostream>
#include <chrono>
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
  std::vector<bstr> pts;
  ph t;
  bool is_set=false;

  void SetUp(::benchmark::State& state) {
    if(!is_set) {
      pts = gen<D, H>(n);
      for(auto pt: pts) t.insert(pt);
      is_set=true;
    }
  }
};

BENCHMARK_TEMPLATE_DEFINE_F(Fixture, knn_query, ${D}, ${H}, ${N})(benchmark::State& state) {
  constexpr u64 D = ${D}, H = ${H};
  int i = 0;
  for (auto _ : state) {
    auto pt=gen<D, H>(1)[0];
    auto start = std::chrono::high_resolution_clock::now();
    auto it = t.knnIterator(pt, false);
    int found=0; for(; !it.end() && found !=100; it++, found++) { }
    std::cout << found << std::endl;
    i = (i + 1)%int(n);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    state.SetIterationTime(found? elapsed_seconds.count()/double(found): elapsed_seconds.count());
    benchmark::DoNotOptimize(it);
  }
}

BENCHMARK_REGISTER_F(Fixture, knn_query)->UseManualTime();
BENCHMARK_MAIN();
