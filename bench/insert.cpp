#include "ph_tree.h"
#include "utils.h"
#include <fstream>
#include <chrono>
#include <utility>
#include <vector>

constexpr u64 D=${D};
constexpr u64 H=${H};
constexpr u64 N=${N};
constexpr u64 seed=0;

using bstr=PHTree<D, H>::bstr;

int main() {
  srand(seed);
  PHTree<D, H> tr;

  auto insert=[&]() {
    auto pt=tr.encode(gen_pt<D, H>());
    auto t1=std::chrono::high_resolution_clock::now();
    tr.insert(pt, nullptr, 0);
    auto t2=std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
  };

  double s=0; for(auto i=0; i!=N; i++) s+=insert();
  std::string file(std::string("insert") + std::string("_") + std::to_string(D) + std::string("_") + std::to_string(H) + std::string("_") + std::to_string(N));
  std::ofstream fout(std::string("benchmarks/insert/") + file);
  fout << int(s/double(N));

  return 0;
}
