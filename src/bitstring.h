#pragma once

#include <cstdint>
#include <algorithm>

using namespace std;
using u64=std::uint64_t;
template <u64 D> struct Range;

template <u64 D> struct Ref {
  static constexpr u64 C=64/D, DM=~-(1ull<<D);
  u64 *a, p1;

  Ref<D> operator=(u64 v) { a[p1/C]=a[p1/C] & ~(DM<<p1%C*D) | (v<<p1%C*D); return *this; }
  Ref<D> operator=(Ref<D> r) { a[p1/C]=a[p1/C] & ~(DM<<p1%C*D) | (r.a[r.p1/C] >> r.p1%C*D << p1%C*D) & (DM<<p1%C*D); return *this; } //careful! check for r.p1==p1, possibly can optimize
  Range<D> operator[](u64 p2) { return Range<D>{a, p1, p2}; }
  Range<D> operator[](u64 p2) const { return Range<D>{a, p1, p2}; }
  operator u64() { return (a[p1/C]>>p1%C*D) & DM; }
};

template <u64 D> struct Range {
  static constexpr u64 C=64/D, DM=~-(1ull<<D);
  u64 *a, p1, p2;
  Range(u64 *_a, u64 _p1, u64 _p2): a(_a), p1(_p1), p2(_p2) { }
  Range(): p1(0), p2(0) { }

  static u64 lm(u64 p) { return 1ull<<p%C*D; }
  //Range<D> operator<<(u64 v0) { u64 i1=p1/C, i2=p2/C, m=lm(p1), b=a[i1] & ~-m; a[i1] &= -m; if((p2+1)%C) a[i2--]<<=D; else a[i2+1]<<=D; for(; i2!=i1-1; i2--) { a[i2+1]|=a[i2]>>(64-D); a[i2]<<=D; } a[i1]|=b; return *this; }
  //Range<D> operator>>(u64 v0) { u64 i1=p1/C, i2=p2/C, m=lm(p1-1); if(p1%C) { a[i1]=a[i1] & ~-m | (a[i1]>>D) & -m; i1++; } for(; i1<=i2; i1++) { a[i1-1]|=a[i1]<<(64-D); a[i1]>>=D; } return *this; }
  Range<D> operator>>(u64 s) { u64 i1=(p1-s)/C, i2=p1/C-s/C, i3=p1/C, i4=p2/C, b=a[i1], m=lm(p1-s), &beg=a[i1]; s=s%C*D; while(i3<=i4) { u64 v=a[i3]; a[i1] |= v<<(64-s); a[i2]=v>>s; i3++; i1=i2; i2++; } beg=b & ~-m | beg & -m; return *this; }
  Range<D> operator<<(u64 s) { u64 sh=s%C*D, k=s/C, i1=p1/C, i2=(p2+s%C)/C, m=lm(p1), b=a[i1] & ~-m; a[i1] &= -m; for(; i2!=i1; i2--) a[i2+k]=a[i2]<<sh | a[i2-1]>>(64-sh); a[i2+k]=a[i2]<<sh; fill(a+i1, a+i1+k, 0); a[i1] |= b; return *this; }
  Range<D> operator=(u64 v) { u64 m1=~-lm(p1), m2=-lm(p2)<<D, b1=a[p1/C] & m1, b2=a[p2/C]; a[p1/C]=b1 | v & ~m1; if(p2/C!=p1/C) { m1=0; std::fill(a+p1/C+1, a+p2/C, v); } a[p2/C]=b2 & m2 | v & ~m2 & ~m1 | (b1 & m1); return *this; }
  Range<D> operator|(u64 v) { u64 m1=~-lm(p1), m2=-lm(p2)<<D, b1=a[p1/C] & m1, b2=a[p2/C]; a[p1/C]|=b1 | v & ~m1; if(p2/C!=p1/C) { m1=0; for(u64 i=p1/C+1; i!=p2/C; i++) a[i]|=v; } a[p2/C]|=b2 & m2 | v & ~m2 & ~m1 | (b1 & m1); return *this; }
  Range<D> operator&(u64 v) { u64 m1=~-lm(p1), m2=-lm(p2)<<D, b1=a[p1/C] & m1, b2=a[p2/C]; a[p1/C]&=b1 | v & ~m1; if(p2/C!=p1/C) { m1=0; for(u64 i=p1/C+1; i!=p2/C; i++) a[i]&=v; } a[p2/C]&=b2 & m2 | v & ~m2 & ~m1 | (b1 & m1); return *this; }
  Range<D> operator=(Range<D> r) { u64 m1=~-lm(p1), m2=-lm(p2)<<D, b1=a[p1/C] & m1, b2=a[p2/C]; a[p1/C]=b1 | r.a[r.p1/C] & ~m1; if(p2/C!=p1/C) { m1=0; std::copy(r.a+r.p1/C+1, r.a+r.p2/C, a+p1/C+1); } a[p2/C]=b2 & m2 | r.a[r.p2/C] & ~m2 & ~m1 | (b1 & m1); return *this; }
  bool operator==(Range<D> r) { u64 m1=-lm(p1); for(u64 i1=p1, i2=r.p1; i1/C!=p2/C; i1+=C, i2+=C, m1=~0ull) if((a[i1/C]^r.a[i2/C]) & m1) return false; return !((a[p2/C]^r.a[r.p2/C]) & m1 & ~-(lm(p2)<<D)); }
};

template <u64 D, u64 DEPTH> struct Bitstring {
  static constexpr u64 C=64/D, DM=~-(1ull<<D);
  u64 a[(DEPTH+C-1)/C]; 

  Ref<D> operator[](u64 pz) { return Ref<D>{a, pz}; }
  Ref<D> operator[](u64 pz) const { return Ref<D>{const_cast<u64*>(a), pz}; }
  bool operator==(const Bitstring<D, DEPTH> &b) { return (*this)[0][DEPTH-1] == b[0][DEPTH-1]; }
};
