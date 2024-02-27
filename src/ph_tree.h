#pragma once
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"

#include <cstdint>
#include <algorithm>
#include <climits>
#include "bitstring.h"
#include "encode.h"
#include <iostream>
#include <vector>
#include <queue>

using u64=std::uint64_t;
using uptr=std::uintptr_t;
using std::swap;
using namespace std;

constexpr void assertions() { static_assert(CHAR_BIT==8); static_assert(alignof(void*) >= 4); static_assert(alignof(void*)==sizeof(void*)); static_assert(alignof(u64*)==sizeof(u64*)); static_assert(sizeof(u64)==sizeof(unsigned long long)); }

template <u64 D0, u64 H>
struct PHTree {
  static constexpr u64 ln[7]={0xffffffffffffffff, 0x5555555555555555, 0x1111111111111111, 0x0101010101010101, 0x0001000100010001, 0x0000000100000001, 0x0000000000000001};
  static constexpr u64 pos(u64 v) { return sizeof(unsigned long long)*8 - __builtin_clzll(v) - 1; }
  static constexpr u64 posu(u64 v) { return pos(2*v-1); }
  static constexpr u64 DK=posu(D0), D=1ull<<DK, C=64ull/D, DM=~-(1ull<<D), POINT_K=(H+63)/64, DH=max(max(D, u64(2)), u64(1)<<posu(pos(H)+1));
  using phtree=PHTree<D, H>;
  using bstr=Bitstring<D, H>;
  using point=Point<D, H>;
  using encoder=Encoder<D, H>;
  enum { LHC=0, AHC=1, INF=2, PSF=3 };

  struct Node;
  struct Iter1D;
  Iter1D stack; bool sign_preprocessor=false, float_preprocessor=false;
  PHTree(bool _sign_preprocessor=false, bool _float_preprocessor=false): stack(), sign_preprocessor(_sign_preprocessor), float_preprocessor(_float_preprocessor) { }
  ~PHTree() { stack.clean(stack.n[0]); }

  //Node layout: 
  //[node_ptrs][sz][ids], types: [void* []][u64][u64 []]
  //sz_ptr*    ^^^ <----points to 
  struct Node {
    u64 t; void *sz_ptr;
    Node(void *ptr): sz_ptr((void*)(uptr(ptr) & -4ull)), t(uptr(ptr) & ~-4ull) { }
    Node(): sz_ptr(0) { }

    static Node alloc(u64 sz, u64 k, u64 t) {
      void *p=aligned_alloc(max(alignof(u64), alignof(void*)), sizeof(u64)*id_n(sz, k, t)+(t==PSF? 0: sizeof(u64)+sizeof(void*)*pt_n(k)));
      p=(void**)p+(t==PSF? 0: pt_n(k)); p=(u64*)p+(t==PSF? -1: 0); Node n((void*)(uptr(p) | t));
      if(t!=PSF) { n.set_sz(sz); n.set_k(t==INF? 0: k); } n.reset_pt();
      return n;
    }
    static void dealloc(Node n) { 
      free(n.t==PSF? (u64*)n.sz_ptr+1: (u64*)n.p()); 
    }
    void *ptr() { return (void*)(uptr(sz_ptr) | t); }
    void reset_pt() { if(t==AHC) fill(p(), p()+(1ull<<D0), nullptr); }

    bool null() { return sz_ptr==0; }
    static u64 id_n(u64 sz, u64 k, u64 t) { return t==AHC? 0: (t==LHC? 1ull<<k: sz); }
    static u64 pt_n(u64 k) { return 1ull<<k; }
    u64 k() { return *(u64*)sz_ptr>>DH+1; }             void set_k(u64 k) { if(t!=PSF) *(u64*)sz_ptr=*(u64*)sz_ptr & ~-(1ull<<DH+1) | (k<<DH+1); }
    u64 sz() { return *(u64*)sz_ptr & ~-(1ull<<DH+1); } void set_sz(u64 sz) { if(t!=PSF) *(u64*)sz_ptr=*(u64*)sz_ptr & -(1ull<<DH+1) | sz; }
    void **p() { return (void**)sz_ptr-pt_n(k()); }
    u64 *id() { return (u64*)sz_ptr+1; }

    u64 lb(u64 p1, u64 p2, u64 v) { for(u64 s=p2-p1+1; s!=0; s/=2) if(v<=(*this)[p1+s/2]) { p2=p1+s/2; } else { p1=p1+(s+1)/2; } return p1; }
    Ref<D> operator[](u64 pz) { return Ref<D>{id(), pz}; }

    static Node shift(Node n, u64 pz, int d) {
      u64 k=n.k(), sz=n.sz(), maxsz=1ull<<k, t=n.t;
      if(d>0) {
        if(sz+1>maxsz) { 
          u64 t2=k+1==D0? AHC: LHC; Node n2=Node::alloc(sz+1, k+1, t2);
          if(t2==AHC) { for(u64 i=0; i!=sz; i++) n2.p()[n[i]]=n.p()[i]; }
          else { copy(n.p(), n.p()+sz, n2.p()); copy(n.id(), n.id()+(sz+C-1)/C, n2.id()); if(pz!=sz) n2[pz][sz-1]<<1; copy_backward(n2.p()+pz, n2.p()+sz, n2.p()+sz+1); }
          Node::dealloc(n); n=n2;
        } else { if(t==LHC) { if(pz!=sz) { n[pz][sz-1]<<1; copy_backward(n.p()+pz, n.p()+sz, n.p()+sz+1); } } n.set_sz(sz+1); }
      } else{
        if(sz-1<=(maxsz/2)) {
          u64 t2=(k-1==0? INF: LHC); Node n2=Node::alloc(sz-1, k-1, t2);
          if(t==AHC) { for(u64 i=0, j=0; i!=1ull<<D0; i++) if(n.p()[i]!=0 && i!=pz-1) { n2.p()[j]=n.p()[i]; n2[j]=i; j++; } }
          else { copy(n.p(), n.p()+pz-1, n2.p()); copy(n.p()+pz, n.p()+sz, n2.p()+pz-1); n[pz-1]=0; if(pz!=sz) n[pz][sz-1]>>1; n2[0][sz-2]=n[0][sz-2]; }
          Node::dealloc(n); n=n2;
        } else { if(t==AHC) { n.p()[pz-1]=0; } else { n[pz-1]=0; if(pz!=sz) n[pz][sz-1]>>1; copy(n.p()+pz, n.p()+sz, n.p()+pz-1); } n.set_sz(sz-1); }
      }
      return n;
    }
  };
  
  struct Iter1D {
    Node n[H]; Bitstring<DH, H> id; u64 pz; bstr path; bool sp;

    Iter1D(bool save_path=false): sp(save_path) { n[0]=Node(0); reset(); }
    Iter1D(const Iter1D &it, bool save_path) { *this = it; sp=save_path; reset(); }

    bstr operator*() { return path; }
    bool operator++(int) { return next(*this); }

    void upd_par(u64 pz) { if(pz!=0) { auto par=n[pz-1]; u64 pk=par.k(); par.p()[par.t==INF? 0: id[pz-1]]=n[pz].ptr(); } }
    u64 di(u64 p1, u64 p2) { return p2-p1/C*C; }

    Node alloc_psf(u64 pz, const bstr &b) { if(pz==H-1) return Node((void*)PSF); Node n2=Node::alloc(di(pz+1, H), 0, PSF); n2[di(pz+1, pz+1)][di(pz+1, H-1)]=b[pz+1][H-1]; return n2; }

    void split(Node p, u64 pz2, const bstr &b) {
      u64 p1=pz, p2=pz2, p3=(p.t==INF? pz+p.sz()-1: H-1);
      if(p1!=p2) {
        Node n2=Node::alloc(di(p1, p2), 0, INF); n2[0][di(p1, p2-1)]=p[0][di(p1, p2-1)]; n2.set_sz(p2-p1);
        id[p2-1]=p2-p1; n[p1]=n[p2-1]=n2; upd_par(p1); }
      Node n2=Node::alloc(2, 1, LHC); u64 id1=p[di(p1, p2)], id2=b[p2], idx1=0, idx2=1;
      if(id1>id2) swap(idx1, idx2);
      n2[idx1]=id1; n2[idx2]=id2; n2.p()[idx1]=p.t==PSF? (void*)PSF: p.p()[0]; n2.p()[idx2]=alloc_psf(p2, b).ptr();
      n[p2]=n2; id[p2]=idx2; upd_par(p2); pz=p2;
      if(p2!=p3) {
        auto a1=Node::alloc(di(p2+1, p3+1), 0, p.t);
        a1[di(p2+1, p2+1)][di(p2+1, p3)]=p[di(p1, p2+1)][di(p1, p3)]; a1.set_sz(p3-p2);
        n2.p()[idx1]=a1.ptr(); if(p.t!=PSF) a1.p()[0]=p.p()[0]; }
      Node::dealloc(p);
    }

    void merge(u64 pos) {
      u64 p1=(pos==0 || n[pos-1].t!=INF? pos: pos-n[pos-1].sz()), p2=pos, p3=pos, p4=pos;
      if(p2!=H-1 && Node(n[p2].p()[0]).t>AHC) { p3=p2+1; n[p2+1]=n[p2].p()[0]; p4=n[p2+1].t==PSF? H-1: p2+n[p2+1].sz(); }
      Node n2=Node::alloc(di(p1, p4+1), 0, p4==H-1? PSF: INF); if(n2.t!=PSF) { n2.set_k(0); n2.set_sz(p4+1-p1); }
      if(p1!=p2) { n2[di(p1, p1)][di(p1, p2-1)]=n[p2-1][di(p1, p1)][di(p1, p2-1)]; Node::dealloc(n[p2-1]); }
      n2[di(p1, p2)]=u64(n[p2][0]);
      if(p3!=p2) { Node n3=Node(n[p2].p()[0]); n2[di(p1, p3)][di(p1, p4)]=n3[di(p3, p3)][di(p3, p4)]; if(n2.t==INF) n2.p()[0]=n3.t==INF? n3.p()[0]: (void*)PSF; Node::dealloc(n3); } else if(p4!=H-1) { n2.p()[0]=n[p2].p()[0]; }
      Node::dealloc(n[p2]); n[p1]=n2; pz=p1; upd_par(p1);
    }

    static u64 cmp(u64 *a, u64 *a2, u64 p1, u64 p2) { p2++; u64 m1=-(1ull<<p1%C*D); for(u64 i=p1/C*C; i<p2; i+=C, m1=~0ull) { u64 bit=(a[i/C] ^ a2[i/C]) & m1; if(bit) { return i+pos(~(bit-1) & bit)/D-p1; } } return p2-p1; }

    void find(const bstr &b) {
      for(pz=0; pz!=H; )
        if(n[pz].t==AHC) { id[pz]=b[pz]; if(n[pz].p()[b[pz]]==0) { return; } else { if(pz+1!=H) n[pz+1]=n[pz].p()[id[pz]]; if(sp) path[pz]=n[pz][id[pz]]; pz++; }
        } else if(n[pz].t==LHC) { id[pz]=n[pz].lb(0, n[pz].sz()-1, b[pz]); if(id[pz]==n[pz].sz() || n[pz][id[pz]]!=b[pz]) return; else { if(pz+1!=H) n[pz+1]=n[pz].p()[id[pz]]; if(sp) path[pz]=id[pz]; pz++; }
        } else { u64 l=n[pz].t==INF? n[pz].sz(): H-pz, bit=cmp(n[pz].id()-pz/C, (u64*)b.a, pz, pz+l-1); if(bit<l) id[pz]=bit; else { if(n[pz].t==INF) n[pz+l]=n[pz].p()[0]; id[pz]=l; id[pz+l-1]=l; n[pz+l-1]=n[pz]; } if(sp) path[pz][pz+l-1]=n[pz][pz%C][l-1]; if(bit<l) break; pz+=l; }
    }

    void insert(const bstr &b) {
      if(n[0].null()) { n[0]=alloc_psf(-1, b); return; }
      find(b);
      if(pz!=H) {
        if(n[pz].t==AHC) { n[pz].p()[id[pz]]=alloc_psf(pz, b).ptr(); n[pz].set_sz(n[pz].sz()+1);
        } else if(n[pz].t==LHC) { auto n2=Node::shift(n[pz], id[pz], 1); if(n2.sz_ptr!=n[pz].sz_ptr) { n[pz]=n2; upd_par(pz); } auto ins=alloc_psf(pz, b).ptr(); if(n[pz].t==LHC) { n[pz][id[pz]]=u64(b[pz]); n[pz].p()[id[pz]]=ins; } else { id[pz]=b[pz]; n[pz].p()[b[pz]]=ins; }
        } else if(pz+id[pz]!=H) { split(n[pz], pz+id[pz], b); }
      }
    }

    bool operator()() { return true; }
    void reset() { pz=0; id[0]=0; if(n[0].null()) pz=-u64(1); }
    void begin() { reset(); if(!end()) down(*this); }
    template <typename Fn> bool next(Fn &filter) { if(up(filter)) down(filter); return !end(); }
    bool end() { return pz==-u64(1); }

    template <typename Fn>
    bool down(Fn &filter) {
      while(pz!=H) {
        if(n[pz].t==LHC) { id[pz]=0; while(!filter()) { id[pz]=id[pz]+1; if(id[pz]==n[pz].sz()) return next(filter); } if(pz+1!=H) n[pz+1]=n[pz].p()[id[pz]]; if(sp) path[pz]=n[pz][id[pz]]; pz++;
        } else if(n[pz].t==AHC) { id[pz]=0; while(n[pz].p()[id[pz]] == 0 || !filter()) { id[pz]=id[pz]+1 & DM; if(id[pz]==0) return next(filter); } if(pz+1!=H) n[pz+1]=n[pz].p()[id[pz]]; if(sp) path[pz]=id[pz]; pz++;
        } else { u64 l=n[pz].t==INF? n[pz].sz(): H-pz; if(!filter()) return next(filter); n[pz+l-1]=n[pz]; if(n[pz].t==INF) n[pz+l]=n[pz].p()[0]; id[pz+l-1]=l; if(sp) path[pz][pz+l-1]=n[pz][pz%C][l-1]; pz+=l; }
      }
      return true;
    }

    template <typename Fn>
    bool up(Fn &filter) {
      auto to_par=[this]() { pz-=(n[pz].t<INF? 1: id[pz]); };
      for(pz=pz-1; pz!=-u64(1); ) {
        if(n[pz].t==LHC) { do { id[pz]=id[pz]+1; if(id[pz]==n[pz].sz()) break; } while(!filter()); if(id[pz]==n[pz].sz()) to_par(); else break;
        } else if(n[pz].t==AHC) { do { id[pz]=id[pz]+1 & DM; if(id[pz]==0) break; } while(n[pz].p()[id[pz]]==0 || !filter()); if(id[pz]==0) to_par(); else break;
        } else { to_par(); }
      }
      if(pz==-u64(1)) return false; else { if(pz!=H-1) n[pz+1]=n[pz].p()[id[pz]]; if(sp) path[pz]=n[pz].t==AHC? id[pz]: n[pz][id[pz]]; pz++; return true; } 
    }

    void remove(const bstr &b) {
      if(n[0].null()) return;
      find(b);
      if(pz!=H) return;
      if(n[pz-1].t==PSF) { Node::dealloc(n[pz-1]); pz-=id[pz-1]+1; } else { pz--; }
      if(pz==-u64(1)) { n[0]=Node((void*)0); return; }
      n[pz]=Node::shift(n[pz], id[pz]+1, -1); upd_par(pz);
      if(n[pz].sz()==1) merge(pz);
    }

    void clean(Node _n) {
      if(_n.null()) return;
      if(_n.t==INF) clean(Node(_n.p()[0]));
      else if(_n.t==LHC) { for(auto i=0; i!=_n.sz(); i++) clean(Node(_n.p()[i]));
      } else if(_n.t==AHC) { for(auto i=0; i!=1ull<<D; i++) if(_n.p()[i]) clean(Node(_n.p()[i])); }
      Node::dealloc(_n);
    }
  };

  struct Iter2D {
    bstr l, r, lm, rm; Iter1D it; bool sp; phtree &tree;

    Iter2D(Iter1D _it, phtree &_tree, bool save_path=true): it{_it}, tree(_tree), sp(save_path) { reset(); }

    bstr operator*() { return it.path; }
    bool operator++(int) { return next(); }
    
    bool inside(u64 v, u64 l, u64 r) { return ((v & r) | l) == v; }

    bool down(u64 *a, u64 p1, u64 p2) { for(; p1<=p2; p1++) if(!down(p1, a[p1/C]>>p1%C*D & DM)) return false; return true; }
    bool down(u64 p1, u64 v) { u64 ll=lm[p1] & l[p1], rr=rm[p1] | r[p1]; if(!inside(v, ll, rr)) return false; if(p1+1!=H) { lm[p1+1]=u64(lm[p1]) & ~((ll^v) & DM); rm[p1+1]=u64(rm[p1]) | ((rr^v) & DM); } return true; }
    bool down_simd(u64 *a, u64 p1, u64 p2) {
      for(u64 p1_m=-(1ull<<p1%C*D), p2_m=~-(1ull<<p2%C*D<<D), mod=p1%C; p1<=p2; p1+=C-mod, p1_m=~0ull, mod=0) {
        u64 ll=lm[p1]*ln[DK] & l.a[p1/C], rr=rm[p1]*ln[DK] | r.a[p1/C], _ll=lm[p1]*ln[DK], _rr=rm[p1]*ln[DK], v=a[p1/C], bl=(ll^v) & p1_m, br=(rr^v) & p1_m;
        for(u64 bit=~((bl | br)-1) & (bl | br); (bl | br)!=0; bit=~((bl | br)-1) & (bl | br))
          if(bit & (bl & ~v | br & v)) { if(p1/C==p2/C && bit & ~p2_m) break; else return 0; }
          else { u64 bln=bit*ln[DK]; if(bl & bit) { _ll &= ~((bit & bl)*ln[DK]); bl &= ~bln; } else { _rr |= (bit & br)*ln[DK]; br &= ~bln; } }
        p1_m<<=D; lm.a[p1/C]=lm.a[p1/C] & ~p1_m | _ll<<D & p1_m; rm.a[p1/C]=rm.a[p1/C] & ~p1_m | _rr<<D & p1_m;
        if(p1-mod+C<H && p1-mod+C<=p2+1) { lm[p1-mod+C]=_ll>>(C-1)*D; rm[p1-mod+C]=_rr>>(C-1)*D; }
      }
      return 1;
    }

    bool operator()() {
      auto n=it.n; auto &id=it.id; auto &pz=it.pz; auto &path=it.path; auto &sp=it.sp;
      if(n[pz].t==LHC) { return down(pz, n[pz][id[pz]]); 
      } else if(n[pz].t==AHC) { return down(pz, id[pz]); 
      } else { u64 ll=lm[pz], rr=rm[pz]; u64 l=n[pz].t==INF? n[pz].sz(): H-pz; return down(n[pz].id()-pz/C, pz, pz+l-1); }
    }
    void reset() { lm[0]=~0ull; rm[0]=0; it.reset(); }
    void begin() { reset(); if(it.end() || !it.down(*this)) it.pz=-u64(1); }
    bool next() { if(it.up(*this)) return it.down(*this); return !end(); }
    bool end() { return it.pz==-u64(1); }

    void set_intersect(const bstr &r) { point d=tree.decode(r), lb, rb; for(u64 i=0; i!=D/2; i++) for(u64 j=0; j!=POINT_K; j++) { lb[i][j]=0ull;    lb[i+D/2][j]=d[i][j];  rb[i][j]=d[i+D/2][j];  rb[i+D/2][j]=~0ull;       } set_rect(tree.encode(lb), tree.encode(rb)); }
    void set_include(const bstr &r)   { point d=tree.decode(r), lb, rb; for(u64 i=0; i!=D/2; i++) for(u64 j=0; j!=POINT_K; j++) { lb[i][j]=d[i][j]; lb[i+D/2][j]=0ull;     rb[i][j]=~0ull;        rb[i+D/2][j]=d[i+D/2][j]; } set_rect(tree.encode(lb), tree.encode(rb)); }
    void set_rect(const bstr &_l, const bstr &_r) { l=_l; r=_r; }
  };

  static double euclid_dist(const point &p1, const point &p2) {  //simple function just for testing
    static_assert(POINT_K <= int(1023/128)); 
    double dist=0; 
    for(auto i=0; i!=D; i++) {
      auto c1=0.0; double exp=1; for(auto k=0; k!=POINT_K; k++) { c1 += double(p1[i][k])*exp; exp *= 1e64; }
      auto c2=0.0;        exp=1; for(auto k=0; k!=POINT_K; k++) { c2 += double(p2[i][k])*exp; exp *= 1e64; }
      dist += (c2 - c1)*(c2 - c1);
    }

    return dist;
  }

  template <double (*dist_fn)(const point &, const point &) = euclid_dist> 
  struct Iter1DKNN { //dijkstra like search: at every step pick node closest to point 
    struct Pt { 
      bstr path; double dist; Node n; u64 mask=0, h=0; Iter1DKNN *it; phtree *tree;

      double get_dist(const Pt &p) { return dist_fn(tree->decode(p.path), it->c); }
      //ineffective?: path update even if no collisions 
      void update(Node to, u64 crd) { u64 cl=(it->bc[h]^crd)&~mask, one=it->bc[h] & cl, zero=~it->bc[h] & cl; mask|=cl; path[h]=crd; if(h+1!=H) { path[h+1][H-1] & ~(zero*ln[DK]); path[h+1][H-1] | one*ln[DK]; } n=to; h++; dist=get_dist(*this); }
      //ineffetive: update for every crd
      void update_long(Node to, u64 len) { auto cn=n; for(u64 p1=h%C, p2=p1+len; p1!=p2; p1++) update(to, cn[p1]); }
    }; 
    struct PtCmp { bool operator()(const Pt &p1, const Pt &p2) { return p1.dist > p2.dist; } };

    point c; bstr bc; phtree &tree;
    std::priority_queue<Pt, std::vector<Pt>, PtCmp> pts;

    Iter1DKNN(const bstr &pivot, Node root, phtree &_tree): pts(PtCmp{}), tree(_tree) { pts.push(Pt{pivot, 0.0, root, u64(0), u64(0), this, &tree}); c=tree.decode(pivot); bc=pivot; next(); }

    void expand(Pt &pt) { 
      if(pt.n.t == PSF) { pt.update_long(Node(nullptr), H-pt.h); pts.push(pt); 
      } else if(pt.n.t == INF) { pt.update_long(Node(pt.n.p()[0]), pt.n.sz()); pts.push(pt); 
      } else if(pt.n.t == LHC) { auto n=pt.n; for(auto i=0; i!=pt.n.sz(); i++) { auto pt2=pt; pt2.update(Node(n.p()[i]), n[i]); pts.push(pt2); }
      } else { auto n=pt.n; for(auto i=0; i!=1ull<<D; i++) { if(n.p()[i]==0) continue; auto pt2=pt; pt2.update(Node(n.p()[i]), i); pts.push(pt2); }
      }
    }

    bool end() { return pts.empty(); }
    void next() { while(!end() && pts.top().h != H) { auto pt=pts.top(); pts.pop(); expand(pt); } }

    void operator++(int) { next(); }
    bstr operator*() { auto pt=pts.top(); pts.pop(); return pt.path; }
  };

  struct IterRay {
    double t[D/2], pt[D/2]; Iter1D it; phtree &tree;

    IterRay(double *_t, double *_pt, Iter1D &_it, phtree &_tree): tree(_tree) { copy(_t, _t+D/2, t); std::copy(_pt, _pt+D/2, pt); it=_it; begin(); }

    bstr operator*() { return it.path; }
    bool operator++(int) { return next(); }

    double to_double(const point &_pt, u64 d) { double c=0, exp=1; for(auto i=0; i!=POINT_K; i++, exp*=1e64) c += _pt[d][i] * exp; return c; }
    bstr bounds(const bstr& curr, u64 h) { bstr ret=curr; u64 mask=~-(1ull << D/2); if(h<H-1) { ret[h+1][H]&0ull; ret[h+1][H]|(mask*ln[DK]); } return ret; }
    bool intersect(const bstr &_rect) {
      point dec=tree.decode(bounds(_rect, it.pz));
      double rectd[D]; for(auto i=0; i!=D; i++) rectd[i]=to_double(dec, i); 

      auto ret=false;
      for(u64 d=0; d!=D/2; d++) 
        if(t[d]!=0) { 
          double t01=(rectd[d]-pt[d])/t[d], t02=(rectd[d+D/2]-pt[d])/t[d]; 
          for(u64 d2=0; d2!=D/2; d2++) { 
            if(d==d2) continue; 
            auto extp1=pt[d2] + t01*t[d2], extp2=pt[d2] + t02*t[d2], eps=0; 
            ret |= (rectd[d2]-eps<=extp1 && extp1<=rectd[d2+D/2]+eps && t01>=0) || (rectd[d2]-eps<=extp2 && extp2<=rectd[d2+D/2]+eps && t02>=0); 
          }
        }
      return ret;
    }

    bool operator()() {
      auto n=it.n; auto &id=it.id; auto &pz=it.pz; auto &path=it.path; auto &sp=it.sp;
      if(n[pz].t==LHC) { path[pz]=n[pz][id[pz]]; return intersect(path); 
      } else if(n[pz].t==AHC) { path[pz]=id[pz]; return intersect(path);
      } else { u64 l=n[pz].t==INF? n[pz].sz(): H-pz; path[pz][pz+l-1]=n[pz][pz%C][l-1]; pz+=l-1; bool i=intersect(path); pz-=l-1; return i; }
    }
    void reset() { it.reset(); }
    void begin() { reset(); if(it.end() || !it.down(*this)) it.pz=-u64(1); }
    bool next() { if(it.up(*this)) return it.down(*this); return !end(); }
    bool end() { return it.pz==-u64(1); }
  };

  Iter1D begin(bool sp=true) { Iter1D it(stack, sp); it.reset(); it.begin(); return it; }

  void insert(const bstr &a, bool sp=false) { stack.sp=sp; stack.reset(); stack.insert(a); }
  void remove(const bstr &a, bool sp=false) { stack.sp=sp; stack.reset(); stack.remove(a); }

  Iter1D pointIterator(const bstr &pt, bool sp=true) { Iter1D it(stack, sp); it.find(pt); if(it.pz != H) it.pz=-u64(1); return it; }
  Iter2D rectIterator(const bstr &l, const bstr &r, bool sp=true) { Iter1D it(stack, sp); Iter2D query(it, *this, sp); query.set_rect(l, r); query.begin(); return query; }
  Iter2D intersectIterator(const bstr &rect, bool sp=true) { Iter1D it(stack, sp); Iter2D query(it, *this, sp); query.set_intersect(rect); query.begin(); return query; }
  Iter2D includeIterator(const bstr &rect, bool sp=true) { Iter1D it(stack, sp); Iter2D query(it, *this, sp); query.set_include(rect); query.begin(); return query; }
  Iter1DKNN<euclid_dist> knnIterator(const bstr &a, bool sp=true) { Iter1DKNN<euclid_dist> it2(a, stack.n[0], *this); return it2; }
  IterRay rayIterator(double *pt, double *t, bool sp=true) { Iter1D it(stack, sp); IterRay it2(t, pt, it, *this); return it2; }
  
  bstr encode(const point &pt) { auto tr=sign_preprocessor? encoder::unsign(pt): pt; auto ret=encoder::morton_encode(tr); encoder::reverse_optimized(ret); return ret; }
  point decode(const bstr &b) { auto ret=b; encoder::reverse_optimized(ret); auto dec=encoder::morton_decode(ret); return sign_preprocessor? encoder::sign(dec): dec; }
};
