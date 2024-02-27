#include <cstdint>
#include <vector>
#include <algorithm>
#include <bitset>
#include <ctime>
#include <iostream>
#include "../encode.h"
#include "include/libmorton/morton.h"
#include "bitstring.h"

using u64=std::uint64_t;
using namespace libmorton;

constexpr u64 D=${D};
constexpr u64 H=${H};
u64 s = time(0);

bool test(u64 n) {
  struct BaseBitstring { 
    std::vector<bool> a;
    BaseBitstring(Bitstring<D, H> &b) { a.resize(H*D); set(b, 0, H-1); } 

    void shift_r(u64 p1, u64 p2, u64 s) { for(u64 i=p1; i!=p2+1; i++) for(u64 d=0; d!=D; d++) a[(i-s)*D+d]=a[i*D+d]; }
    void shift_l(u64 p1, u64 p2, u64 s) { a.resize(std::max((p2+s+1)*D, a.size())); for(u64 i=p2; i!=p1-1; i--) for(u64 d=D-1; d!=-u64(1); d--) a[(i+s)*D+d]=a[i*D+d]; for(u64 i=0; i!=s; i++) for(u64 d=D-1; d!=-u64(1); d--) a[(p1+i)*D+d]=0; }
    void set(Bitstring<D, H> &b, u64 p1, u64 p2) { for(u64 i=p1; i!=p2+1; i++) { u64 m=b[i]; for(u64 d=0; d!=D; d++) a[i*D+d]=(m>>d) & 1ull; } }
    void _and(Bitstring<D, H> &b, u64 p1, u64 p2) { for(u64 i=p1; i!=p2+1; i++) { u64 m=b[i]; for(u64 d=0; d!=D; d++) a[i*D+d]=a[i*D+d] & (m>>d) & 1ull; } }
    void _or(Bitstring<D, H> &b, u64 p1, u64 p2) { for(u64 i=p1; i!=p2+1; i++) { u64 m=b[i]; for(u64 d=0; d!=D; d++) a[i*D+d]=a[i*D+d] | (m>>d) & 1ull; } }
    void print(u64 p1, u64 p2) { for(u64 i=p1; i!=p2+1; i++) { std::cout << " " << i << ": "; for(u64 d=0; d!=D; d++) { std::cout << u64(a[D*i+d]); } std::cout << " "; } std::cout << std::endl; }
    bool eq(BaseBitstring b, u64 p1, u64 p2) { for(u64 i=p1; i!=p2+1; i++) for(u64 d=0; d!=D; d++) if(a[i*D+d]!=b.a[i*D+d]) return false; return true; }
  };
  using bstr1=BaseBitstring;
  using bstr2=Bitstring<D, H>;

  constexpr u64 C=64/D, k=(H+C-1)/C; 
  for(u64 i=0; i!=n; i++) {
    bstr2 gen; for(u64 i=0; i!=H; i++) gen[i]=rand()%(1ull<<D);

    u64 p1=rand()%H, p2=p1+rand()%(H-p1), sr=p1==0? 0: rand()%(p1+1), sl=rand()%(H-p2);
    if(sr%C) {
      bstr1 b1(gen); bstr2 b2=gen;
      b1.shift_r(p1, p2, sr);
      b2[p1-sr][p1-1]=0;
      b2[p1][p2]>>sr;
      if(!b1.eq(b2, 0, p2-sr)) return false; 
    }
    if(sl%C) {
      bstr1 b1(gen); bstr2 b2=gen;
      b1.shift_l(p1, p2, sl);
      b2[p1][p2]<<sl;
      if(!b1.eq(b2, 0, p2+sl)) return false;
    }
    
    bstr2 gen2; for(u64 i=0; i!=H; i++) gen2[i]=rand()%(1ull<<D);
    bstr2 b2=gen; b2[p1][p2]=gen2[p1][p2];
    if(!bstr1(gen2).eq(bstr1(b2), p1, p2)) { return false; }
    if(p1!=0) { if(!bstr1(gen).eq(bstr1(b2), 0, p1-1)) { return false; } }
    if(p2!=H-1) { if(!bstr1(gen).eq(bstr1(b2), p2+1, H-1)) { return false; } }

    u64 v1=rand()%(1ull<<D); bstr2 anda; for(u64 i=0; i!=H; i++) anda[i]=v1;
    bstr2 and1=gen; and1[p1][p2] & anda[i].a[0];
    bstr1 and2(gen); and2._and(anda, p1, p2);
    if(!and2.eq(and1, p1, p2)) { return false; }

    u64 v2=rand()%(1ull<<D); bstr2 ora; for(u64 i=0; i!=H; i++) ora[i]=v2;
    bstr2 or1=gen; or1[p1][p2] | ora[i].a[0];
    bstr1 or2(gen); or2._or(ora, p1, p2);
    if(!or2.eq(or1, p1, p2)) { return false; }

    bstr2 g1=gen, g2=gen;
    u64 pz=rand()%H; g2[pz]=u64(g2[pz])^(rand()%2);
    auto cmp1=bstr1(g1).eq(bstr1(g2), 0, H-1);
    auto cmp2=g1[0][H-1]==g2[0][H-1];
    if(cmp1!=cmp2) return false; 
  }

  return true;
}

int main() {
  std::cout << "seed: " << s << std::endl;
  srand(s);
  auto succ=test(1e3);
  return succ? 0: -1;
}
