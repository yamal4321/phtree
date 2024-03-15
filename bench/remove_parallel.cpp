#include "ph_tree_parallel.h"
#include "utils.h"
#include <fstream>
#include <chrono>
#include <utility>
#include <vector>

constexpr u64 D=${D};
constexpr u64 H=${H};
constexpr u64 N=${N};
constexpr u64 C=${C};
constexpr u64 seed=0;

using bstr=PHTreeParallel<D, H>::bstr;

int main() {
  srand(seed);

  auto insert=[&]() {
    PHTreeParallel<D, H> tr;
    for(auto i=0; i!=N; i++) tr.insert(tr.encode(gen_pt<D, H>()), nullptr, 0);
    std::vector<bstr> pts; auto it=tr.begin(); for(auto i=0; i!=1e4 && !it.end(); i++, it++) pts.push_back(*it);

    auto t1=std::chrono::high_resolution_clock::now();
    tr.remove(pts, C);
    auto t2=std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count()/double(pts.size());
  };

  double s=0; for(auto i=0; i!=1e1; i++) s+=insert()*1e-1;
  std::string file(std::string("remove_parallel") + std::string("_") + std::to_string(D) + std::string("_") + std::to_string(H) + std::string("_") + std::to_string(N) + std::string("_") + std::to_string(C));
  std::ofstream fout(std::string("benchmarks/remove_parallel/") + file);
  fout << int(s);

  return 0;
}
