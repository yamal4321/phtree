#include "encode.h"
#include "ph_tree_parallel.h"
#include <cstdint>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <bitset>
#include <vector>
#include <functional>
#include <map>
#include <chrono>

using u64=std::uint64_t;
using uptr=uintptr_t;
using namespace libmorton;

constexpr u64 D=${D};
constexpr u64 H=${H};

u64 s = time(0);
using bstr=Bitstring<D, H>;

struct BasePHTree {
  struct Node { Node *nodes[1ull << D]; Node() { std::fill(nodes, nodes+(1ull << D), nullptr); }  bool empty() { for(auto i=0; i!=(1ull << D); i++) if(nodes[i]) return false; return true; } };

  Node *root;
  BasePHTree() { root = new Node(); }
  
  void clean(Node *curr) { if(curr == 0) return; for(auto i=0; i!=(1ull << D); i++) clean(curr->nodes[i]); delete curr; }
  ~BasePHTree() { clean(root); }

  void insert(const bstr &b) {
    auto curr = root;
    for(auto i=0; i != H; i++) {
      if(curr->nodes[b[i]] == nullptr) curr->nodes[b[i]] = new Node();
      curr = curr->nodes[b[i]];
    }
  }

  void remove(const bstr &b, Node *from, u64 i=0) {
    if(i == H) return;
    if(from->nodes[b[i]] == nullptr) return;
    remove(b, from->nodes[b[i]], i+1);
    if(from->nodes[b[i]]->empty()) { delete from->nodes[b[i]]; from->nodes[b[i]]=nullptr; }
  }

  void remove(const bstr &b) { remove(b, root, 0); }

  void traverse(bstr path, Node *curr, u64 i, std::vector<bstr> &ret) {
    if(i==H) { ret.push_back(path); return; } 
    for(auto k=0; k!=(1ull << D); k++) {
      if(curr->nodes[k] == nullptr) continue;
      path[i]=k;
      traverse(path, curr->nodes[k], i+1, ret);
    }
  }

  std::vector<bstr> traverse() { std::vector<bstr> ret; traverse(bstr(), root, 0, ret); return ret; }

};

bool test(u64 n) {
  using ph1=BasePHTree;
  using ph2=PHTreeParallel<D, H>;
  u64 POINT_K = ph2::POINT_K;
  using bstr=ph2::bstr;
  using pt=Point<D, H>;

  auto hash=[&](pt p) { u64 val=0; for(auto i=0; i!=D; i++) for(auto j=0; j!=POINT_K; j++) val+=p[i][j]; return (void*)uptr(val); };

  static_assert(sizeof(decltype(rand())) >= 4);
  std::vector<pt> gen;
  for(auto i=0; i!=n; i++) {
    pt p; for(auto j=0; j!=D; j++) for(auto k=0; k!=pt::POINT_K; k++) p.a[j][k] = ((u64)rand() << 32) | rand();
    gen.push_back(p);
  }

  ph1 t1;
  ph2 t2;

  auto traverse1 = [&](ph1 &p) { return p.traverse(); };
  auto traverse2 = [&](ph2 &p) { std::vector<bstr> ret; for(auto it=p.begin(); !it.end(); it++) ret.push_back(*it); return ret; };
  auto ptrs = [&](ph2 &p) { std::vector<void*> ret; for(auto it=p.begin(); !it.end(); it++) ret.push_back(it.ptr()); return ret; };
  auto check_ptrs = [&](ph2 &p) { auto pts=traverse2(p); auto ps=ptrs(p); bool eq=1; for(auto i=0; i!=pts.size(); i++) { auto h1=ps[i], h2=hash(p.decode(pts[i])); eq &= h1==h2; } return eq; };
  auto gen_ptrs =[&](std::vector<bstr> &v) { std::vector<void*> ret; for(auto &pt: v) ret.push_back(hash(t2.decode(pt))); return ret; };

  auto nthreads = std::thread::hardware_concurrency();
  std::vector<bstr> enc; for(auto pt: gen) enc.push_back(t2.encode(pt));

  for(auto p: enc) { t1.insert(p); } t2.insert(enc, gen_ptrs(enc), nthreads);
  auto ins1=traverse1(t1), ins2=traverse2(t2);
  bool ins1_eq=true;
  if(ins1.size() != ins2.size()) ins1_eq=0; else for(auto i=0; i!=ins1.size(); i++) { ins1_eq &= (ins1[i] == ins2[i]); }

  std::vector<bstr> rem=enc; 
  std::random_shuffle(rem.begin(), rem.end());
  rem = std::vector<bstr>(rem.begin(), rem.begin() + rem.size()/2);
  for(auto p: rem) { t1.remove(p); } t2.remove(rem, nthreads);
  auto rem1=traverse1(t1), rem2=traverse2(t2);
  bool rem_eq=true;
  if(rem1.size() != rem2.size()) rem_eq=0; else for(auto i=0; i!=rem1.size(); i++)  rem_eq &= (rem1[i] == rem2[i]);

  std::vector<bstr> ins(enc.begin()+enc.size()/2, enc.end());
  for(auto p: ins) { t1.insert(p); } t2.insert(ins, gen_ptrs(ins), nthreads);
  auto ins3=traverse1(t1), ins4=traverse2(t2);
  bool ins2_eq=true;
  if(ins3.size() != ins4.size()) ins2_eq=0; else for(auto i=0; i!=ins3.size(); i++)  ins2_eq &= (ins3[i] == ins4[i]);

  bool ptrs_eq=check_ptrs(t2);
  return ins1_eq & rem_eq & ins2_eq & ptrs_eq;
}

int main() {
  std::cout << "seed: " << s << std::endl;
  srand(s);
  auto succ=test(3e4);
  return succ? 0: -1;
}
