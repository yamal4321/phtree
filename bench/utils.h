#pragma once
#include "ph_tree.h"
#include <cstddef>
#include <algorithm>
#include <vector>

using u64 = std::uint64_t;
using u32 = std::uint32_t;

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
    for(int j=ph::POINT_K-1; j!=-1; j--) {
      if(ret[d][j]<ret[d+D/2][j]) break;
      if(ret[d][j]>ret[d+D/2][j]) { for(auto i=0; i!=ph::POINT_K; i++) std::swap(ret[d][i], ret[d+D/2][i]); break; } 
    }
  return ret;
}

template <u64 D, u64 H, bool use_simd=false> PHTree<D, H, use_simd> gen_tree(int n, bool use_rect=false) {
  PHTree<D, H, use_simd> tr;
  for(auto i=0; i!=n; i++) tr.insert(tr.encode(!use_rect? gen_pt<D, H>(): gen_rect<D, H>()), nullptr, 0);
  return tr;
} 
