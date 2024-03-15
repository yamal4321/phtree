#include "ph_tree.h"
#include "utils.h"
#include <fstream>
#include <chrono>
#include <utility>

constexpr u64 D=${D};
constexpr u64 H=${H};
constexpr u64 N=${N};
constexpr u64 seed=0;

int main() {
  srand(seed);
  auto tr=gen_tree<2*D, H>(N, true);
  u64 found = 0;

  auto rect_query=[&]() {
    auto pt=tr.encode(gen_rect<2*D, H>());

    auto t1=std::chrono::high_resolution_clock::now();
    auto it=tr.intersectIterator(pt, 0);
    int i=0; for(; !it.end(); i++, it++) { }
    auto t2=std::chrono::high_resolution_clock::now();

    found += i;
    return i<=10? 1e12: std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count()/double(i);
  };

  double s=0, k=0; for(auto i=0; i!=1e3; i++) { auto t=rect_query(); if(t==1e12) continue; s+=t; k++; if(found >= 1e5) break; }
  std::string file(std::string("intersect_query") + std::string("_") + std::to_string(D) + std::string("_") + std::to_string(H) + std::string("_") + std::to_string(N));
  std::ofstream fout(std::string("benchmarks/intersect_query/") + file);
  fout << int(s/k);

  return 0;
}
