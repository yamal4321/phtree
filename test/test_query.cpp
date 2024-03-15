#include "ph_tree.h"
#include <vector>
#include <algorithm>
#include <cstdint>

using u64=std::uint64_t;
using u32=std::uint32_t;

u64 seed=0;

constexpr u64 D=${D};
constexpr u64 H=${H};

template <u64 D, u64 H> typename PHTree<D, H>::point gen_pt() {
  using ph=PHTree<D, H>;
  typename ph::point ret;
  for(auto d=0; d!=D; d++) 
    for(auto i=0; i!=ph::POINT_K; i++) 
      ret[d][i]= (u64(rand()) << 32) | u32(rand());
  return ret;
}

template <u64 D, u64 H> typename PHTree<D, H>::point gen_rect() {
  using ph=PHTree<D, H>;
  auto ret=gen_pt<D, H>();
  for(auto d=0; d!=D/2; d++)
    for(auto j=ph::POINT_K-1; j!=-1; j--) {
      if(ret[d][j]<ret[d+D/2][j]) break;
      if(ret[d][j]>ret[d+D/2][j]) { for(auto i=0; i!=ph::POINT_K; i++) std::swap(ret[d][i], ret[d+D/2][i]); break; } 
    }
  for(auto d=0; d!=D; d++)
  if(H%64) ret[d][ph::POINT_K-1] &= ~-(1ull << H%64);
  return ret;
}

using ph=PHTree<2*D, H, true>;
using pt=typename ph::point;
using uptr=std::uintptr_t;

int main() {
  seed=time(0);
  srand(seed);
  std::cout << "seed: " << seed << std::endl;

  auto leq=[](u64 *a1, u64 *a2) { for(auto i=0; i!=ph::POINT_K; i++) { auto c1=a1[ph::POINT_K-i-1], c2=a2[ph::POINT_K-i-1]; if(c1>c2) return 0; if(c1<c2) return 1; } return 1; };
  auto intersect = [&](pt r1, pt r2) { bool ret=1; for(u64 a=0, b=D; a!=D; a++, b++) { ret &= leq(r1[a], r2[b]); ret &= leq(r2[a], r1[b]); } return ret; };

  auto rq=gen_rect<2*D, H>();
  std::vector<pt> pts; 
  ph tr;
  for(auto i=0; i!=1e5; i++) { auto p=gen_rect<2*D, H>(); tr.insert(tr.encode(p), (void*)i); pts.push_back(p); }

  std::vector<int> to_sort1, to_sort2;
  for(auto it=tr.begin(); !it.end(); it++) if(intersect(rq, tr.decode(*it))) to_sort1.push_back(uptr(it.ptr())); 
  for(auto it=tr.intersectIterator(tr.encode(rq)); !it.end(); it++) to_sort2.push_back(uptr(it.ptr()));

  std::sort(to_sort1.begin(), to_sort1.end());
  std::sort(to_sort2.begin(), to_sort2.end());

  if(to_sort1.size() != to_sort2.size()) return 1;
  for(auto i=0; i!=to_sort1.size(); i++) if(to_sort1[i]!=to_sort2[i]) return 1;
  return 0;
}
