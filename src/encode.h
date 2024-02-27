#pragma once

#include "libmorton/include/libmorton/morton.h"
#include <cstdint>
#include <algorithm>
#include "bitstring.h"
#include <cmath>

template <u64 D, u64 H>
struct Point {
    static constexpr u64 C=64/D;
    static constexpr u64 POINT_K=(H+63)/64;
    u64 a[D][POINT_K]; 

    bool operator==(Point pt) { for(u64 j=0; j!=D; j++) { if(!(Ref<1>{(u64*)a+j*POINT_K, 0}[H-1]==Ref<1>{(u64*)pt.a+j*POINT_K, 0}[H-1])) return false; } return true; }
    u64 *operator[](u64 pz) { return a[pz]; };
    const u64 *operator[](u64 pz) const { return a[pz]; };
    bool operator!=(Point pt) { return !((*this)==pt); }
};

template <u64 D, u64 H> struct Encoder {
  using u64=std::uint64_t;
  using u32=std::uint32_t;
  using bstr=Bitstring<D, H>;
  using point=Point<D, H>;

  static constexpr u64 C=64/D;
  static constexpr u64 POINT_K=(H+63)/64;

  static bstr encode(const point &pt) {
    bstr ret;
    for(u64 i=0; i!=H; i++) {
      u64 m=0, k=i/64;
      for(u64 j=0; j!=D; j++) m |= ((pt.a[j][k]>>i%64) & 1ull)<<j;
      ret[i]=m;
    }
    return ret;
  }

  static point decode(const bstr &b) {
    point pt; std::fill((u64*)pt.a, (u64*)pt.a+D*POINT_K, 0);
    for(u64 i=0; i!=H; i++) {
      u64 k=i/64, m=b[i];
      for(u64 j=0; j!=D; j++) pt.a[j][k] |= ((m>>j) & 1ull)<<(i%64);
    }
    return pt;
  }

  static bstr morton_encode(const point &pt) {
    u64 buf[D]; bstr ret;
    for(u64 k=0; k!=POINT_K; k++) {
      for(u64 i=0; i!=D && k*64+i*C<H; i++) {
        for(u64 d=0; d!=D; d++) buf[d]=pt.a[d][k]>>i*(C%64);
        for(u64 sz=D; sz!=1; sz>>=1) for(u64 pz=0; pz!=sz/2; pz++) buf[pz]=libmorton::morton2D_64_encode(buf[pz], buf[pz+sz/2]);
        ret.a[k*D+i]=buf[0];
      }
    }
    return ret;
  }

  static bstr morton_encode_optimized(const point &pt) {
    u64 buf[2*D]; bstr ret; 
    for(u64 k=0, dk=std::log2(D), h=0, sz=1ull<<dk, pz=0; k!=POINT_K; k++, sz=1ull<<dk, pz=0, h=0) {
      for(u64 i=0; i!=D; i++) buf[i]=pt.a[i][k];
      for(u64 i=0; i!=D && k*64+i*C<H; ) {
        for(; h!=dk; pz+=sz, h++, sz>>=1) for(u64 j=0; j!=sz/2; j++) buf[pz+sz+j]=libmorton::morton2D_64_encode(buf[pz+j], buf[pz+j+sz/2]);
        ret.a[k*D+i]=buf[2*D-2];
        i++; u64 bit=~(i-1ull) & i; while(bit%D) { sz*=2; h--; pz-=sz; bit>>=1; }
        for(u64 j=0; j!=sz; j++) buf[pz+j]>>=32;
      }
    }
    return ret;
  }

  static point morton_decode(const bstr &b) {
    u64 buf[D]; point pt; std::fill((u64*)pt.a, (u64*)pt.a+D*POINT_K, 0);
    for(u64 k=0; k!=POINT_K; k++) {
      for(u64 i=0; i!=D && k*64+i*C<H; i++) {
        buf[0]=b.a[k*D+i];
        for(u64 sz=1; sz!=D; sz<<=1) for(u64 pz=0; pz!=sz; pz++) { uint_fast32_t h1, h2; libmorton::morton2D_64_decode(buf[pz], h1, h2); buf[pz]=h1; buf[pz+sz]=h2; }
        for(u64 d=0; d!=D; d++) pt.a[d][k] |= buf[d]<<i*(C%64);
      }
    }
    return pt;
  }

  static void encode(u64 *__restrict__ pt, u64 *__restrict__ to) { *(bstr*)(to)=encode(*(point*)pt); }
  static void morton_encode(u64 *__restrict__ pt, u64 *__restrict__ to) { *(bstr*)(to)=morton_encode(*(point*)pt); }
  static void morton_encode_optimized(u64 *__restrict__ pt, u64 *__restrict__ to) { *(bstr*)(to)=morton_encode_optimized(*(point*)pt); }
  static void morton_decode(u64 *__restrict__ b, u64 *__restrict__ to) { *(point*)to=morton_decode(*(bstr*)b); }

  static point unsign(const point &p) { auto ret=p; u64 s=1ull<<H-1; for(auto i=0; i!=D; i++) ret[i][0]=p[i][0]&s? ~(p[i][0]-1): p[i][0]|s; return ret; }
  static point sign(const point &p) { auto ret=p; u64 s=1ull<<H-1; for(auto i=0; i!=D; i++) ret[i][0]=p[i][0]&s? p[i][0]^s: ~p[i][0]+1; return ret; }

  static point to_int(const point &p) { 
    auto ret=p; u64 s=1ull<<H-1; 
    for(auto i=0; i!=D; i++) { ret[i][0]=ret[i][0]^1; }
    return ret;
  }
  static point to_float(const point &p) { }
#define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
  static u32 reverse(u32 num) { u32 t[256]={R6(0), R6(2), R6(1), R6(3)}; return t[num & 255]<<24 | t[(num>>8) & 255]<<16 |  t[(num>>16) & 255]<< 8 | t[(num>>24) & 255] ; }
  static u64 reverse64(u64 num) { u64 n1=reverse(u32(num)), n2=reverse(u32(num>>32)); return n2 | (n1<<32); }

  static void reverse_optimized(bstr &b) { u64 n=(H+C-1)/C; for(u64 i=0; i!=n; i++) b.a[i]=reverse64(b.a[i]); std::reverse(b.a, b.a+n); u64 sh=C-H%C; if(sh%C) b[sh][H-1]>>sh; }
  static void reverse(bstr &b) { bstr ret; for(u64 i=0; i!=H; i++) { u64 m1=b[i], m2=0; for(u64 j=0; j!=D; j++) m2 |= (m1>>j & 1ull)<<(D-j-1); ret[H-i-1]=m2; } b=ret; }
};
