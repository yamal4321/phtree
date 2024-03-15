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
  auto tr=gen_tree<D, H>(N);
  std::vector<bstr> pts; auto it=tr.begin(); for(auto i=0; i!=1e4 && !it.end(); i++, it++) pts.push_back(*it);

  auto remove=[&](bstr &pt) {
    auto t1=std::chrono::high_resolution_clock::now();
    tr.remove(pt);
    auto t2=std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
  };

  double s=0; for(auto i=0; i!=1e4 && i<pts.size(); i++) s+=remove(pts[i])/double(pts.size());
  std::string file(std::string("remove") + std::string("_") + std::to_string(D) + std::string("_") + std::to_string(H) + std::string("_") + std::to_string(N));
  std::ofstream fout(std::string("benchmarks/remove/") + file);
  fout << int(s);

  return 0;
}
