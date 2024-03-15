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
  auto tr=gen_tree<D, H>(N);

  auto knn_query=[&](int n) {
    auto pt=tr.encode(gen_pt<D, H>());

    auto t1=std::chrono::high_resolution_clock::now();
    auto it=tr.knnIterator(pt, 0);
    int i=0; for(; !it.end() && i!=n; i++, it++) { }
    auto t2=std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count()/double(i);
  };

  double s=0; for(auto i=0; i!=1e3; i++) s+=knn_query(1500)*1e-3;
  std::string file(std::string("knn_query") + std::string("_") + std::to_string(D) + std::string("_") + std::to_string(H) + std::string("_") + std::to_string(N));
  std::ofstream fout(std::string("benchmarks/knn_query/") + file);
  fout << int(s);

  return 0;
}
