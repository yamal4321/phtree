#include <cstdint>
#include <ctime>
#include <algorithm>
#include <bitset>
#include <iostream>
#include "libmorton/include/libmorton/morton.h"
#include "bitstring.h"
#include "../encode.h"

using u64=std::uint64_t;
using namespace libmorton;

constexpr u64 D=${D};
constexpr u64 H=${H};
u64 s = time(0);

bool test(u64 n) {
    using encoder=Encoder<D, H>; 
    using point=Point<D, H>;
    using bstr = Bitstring<D, H>;
    auto gen=[&]() { point p; for(u64 k=0; k!=encoder::POINT_K; k++) { for(u64 j=0; j!=D; j++) p.a[j][k]=u64(rand())<<32 | u64(rand()); } return p; };

    for(u64 i=0; i!=n; i++) {
      auto pt1=gen();
      auto e1=encoder::encode(pt1), e2=encoder::morton_encode(pt1);
      auto d1=encoder::decode(e1), d2=encoder::morton_decode(e2);
      auto r1=e1, r2=e2; encoder::reverse(r1); encoder::reverse_optimized(r2);
      if(!(d1==d2)) return false;
      if(!(r1==r2)) return false;
      if(!(e1==e2)) return false; 
    }
    return true;
}

int main() {
  std::cout << "seed: " << s << std::endl;
  srand(s);
  auto succ=test(1e4);
  return succ? 0: -1;
}
