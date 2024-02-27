#include <cstdint>
#include <iostream>
#include <algorithm>
#include <numeric>

using namespace std;
using u64=std::uint64_t;
constexpr u64 D=${D};
u64 s=time(0);

bool inside(u64 v, u64 l, u64 r) { return (v & r | l) == v; }

u64 make_l() { return rand() & ~-(1ull << D); }
u64 make_r(u64 l) { return l | make_l(); }

u64 next(u64 v, u64 l, u64 r) {  return ((v | ~r) + 1) & r | l; }

int test() {
  auto l=make_l(), r=make_r(l);
  if((r | l) != r) return false;

  bool is_valid[1ull<<D]; 
  bool is_set[1ull<<D]; std::fill(is_set, is_set+(1ull<<D), 0);
  for(u64 i=0; i!=1ull<<D; i++) is_valid[i]=inside(i, l, r);

  for(u64 i=0, v=l; i!=1ull<<D; i++, v=next(v, l, r)) if(!inside(v, l, r)) return 1; else is_set[v]=1;
  for(u64 i=0; i!=1ull<<D; i++) if(is_set[i]!=is_valid[i]) return 1;

  return 0;
}

int main() {
  std::cout << "seed: " << s << std::endl;
  srand(s);
  u64 ret=0;
  for(auto i=0; i!=1e5; i++) ret |= test();
  return ret;
}
