#include "../ph_tree.h"
#include "../bitstring.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

using tree=PHTree<${D}, ${H}>; using node=tree::Node;
using namespace std;
u64 s=time(0);

static constexpr u64 pos(u64 v) { return sizeof(unsigned long long)*8 - __builtin_clzll(v) - 1; }
static constexpr u64 posu(u64 v) { return pos(2*v-1); }

bool test_powers() {
  u64 pow[65], powu[65];
  for(auto i=1; i!=65; i++) { pow[i]=std::log2f(i); powu[i]=std::ceil(std::log2f(i)); }
  bool eq=true;
  for(auto i=1; i!=65; i++) { eq &= pow[i]==pos(i); eq &= powu[i]==posu(i); }
  return eq;
}

bool test_sz() {
  bool eq=true;
  for(auto i=0; i!=1e3; i++) {
    u64 sz=${D}==1? 2: rand()%((1ull<<${D}) - 2) + 2, k=posu(sz), sz2=sz+1, k2=k==${D}? k: k+1;
    auto n=node::alloc(sz, k, k==1ull<<${D}? tree::AHC: tree::LHC);
    n.set_sz(sz2); n.set_k(k2);
    eq &= n.sz()==sz2; eq &= n.k()==k2; eq &= n.t==(k==1ull<<${D}? tree::AHC: tree::LHC);
    n.set_sz(sz); n.set_k(k);
    node::dealloc(n);
  }
  return eq;
}

bool test_lb() {
  bool eq=true;
  for(auto i=0; i!=1e3; i++) {
    u64 sz=${D}==1? 2: rand()%((1ull<<${D}) - 2) + 2, k=posu(sz); std::vector<u64> v(sz); std::vector<node> nodes(sz);
    auto n=node::alloc(sz, k, k==${D}? tree::AHC: tree::LHC);
    if(n.t==tree::AHC) { node::dealloc(n); continue; }
    for(u64 i=0; i!=sz; i++) { v[i]=i; nodes[i]=node::alloc(2, 1, 0); }
    for(u64 i=0; i!=sz/5; i++) { v.erase(v.begin()+i); node::dealloc(nodes[i]); nodes.erase(nodes.begin()+i); }
    sz-=sz/5;
    for(auto i=0; i!=sz; i++) { n[i]=v[i]; n.p()[i]=nodes[i].ptr(); }
    auto lb=[&](int p1, int p2, int val) { for(auto i=p1; i!=p2+1; i++) { if(v[i]>=val) return i; } return p2+1; };

    for(auto i=0; i!=100; i++) {
      auto p1=rand()%sz, p2=p1+rand()%(sz-p1), val=v[rand()%sz];
      auto lb1=lb(p1, p2, val);
      auto lb2=n.lb(p1, p2, val);
      eq &= lb1==lb2; 
      if(eq && (lb1>=p1) && (lb1<=p2)) { eq &= n.p()[lb1]==nodes[lb1].ptr(); }
    }
    for(auto i=0; i!=sz; i++) node::dealloc(nodes[i]);
    node::dealloc(n);
  }
  return eq;
}

bool test_bit_operations() {
  static u64 C=64/${D};
  struct Bitstring_raw {
    std::vector<char> data;
    Bitstring_raw(u64 *a, u64 p1, u64 p2) { data=std::vector<char>(p1*${D}, '_'); for(auto i=p1; i!=p2+1; i++) append(a[i/C], i%C); for(auto i=0; i!=${D}; i++) data.push_back('_'); }
    Bitstring_raw(u64 v, u64 pz) { append(v, pz); }
    Bitstring_raw() { }
    void append(u64 v, u64 pz) { v=v>>(pz*${D}) & ~-(1ull<<${D}); for(auto i=0; i!=${D}; i++, v>>=1) data.push_back(v&1? '#': '_'); }
    void print() { for(auto i=0; i!=data.size(); i++) { if(i!=0 && i%${D}==0) std::cout << " "; std::cout <<data[i]; } std::cout << std::endl; }
    void shift_l(int p1, int p2, int sh) { for(auto i=(p2+1)*${D}-1; i!=p1*${D}-1; i--) data[i+sh*${D}]=data[i]; for(auto i=p1*${D}; i!=p1*${D}+sh*${D}; i++) data[i]='_'; }
    void shift_r(int p1, int p2, int sh) { for(auto i=p1*${D}; i!=(p2+1)*${D}; i++) data[i-sh*${D}]=data[i]; }
    void reset(int p1, int p2) { for(auto i=p1*${D}; i!=(p2+1)*${D}; i++) data[i]='_'; }
    void set(u64 *a, int p1, int p2) { for(auto i=p1*${D}; i!=(p2+1)*${D}; i++) data[i]=((1ull << i%64) & a[i/64])? '#': '_'; }
    void set(u64 v, int p1, int p2) { for(auto i=p1*${D}; i!=(p2+1)*${D}; i++) data[i]=((1ull << i%64) & v)? '#': '_'; }
    bool equal(Bitstring_raw b, int p1, int p2) { for(auto i=p1*${D}; i!=(p2+1)*${D}; i++) if(data[i]!=b.data[i]) return 0; return 1; }
  };

  bool eq=true;
  u64 a[((1ull<<${D})+C-1)/C];
  for(auto i=0; i!=1e5; i++) {
    u64 sz=max(2ull, rand()%(1ull<<${D})), p1=rand()%sz, p2=p1+rand()%(sz-p1), k=posu(sz);
    for(auto j=0; j!=((1ull<<${D})+C-1)/C; j++) a[j]=u64(rand()) << 32 | rand();
    if(p2==sz-1) continue;
    auto n=node::alloc(sz, k, k==${D}? tree::AHC: tree::LHC);
    if(n.t==tree::AHC) { node::dealloc(n); continue; }

    //copy
    n[p1][p2]=Ref<${D}>{a, p1}[p2];
    Bitstring_raw b1(a, p1, p2);
    Bitstring_raw b2(n.id(), p1, p2);
    eq &= b1.equal(b2, p1, p2);

    //shift_l
    n[p1][p2]<<1;
    b1.shift_l(p1, p2, 1);
    b2=Bitstring_raw(n.id(), p1, p2+1);
    eq &= b1.equal(b2, p1, p2+1);

    //shift_r
    n[p1+1][p2+1]>>1;
    b1.shift_r(p1+1, p2+1, 1);
    b2=Bitstring_raw(n.id(), p1, p2);
    eq &= b1.equal(b2, p1, p2);

    //set
    for(auto j=0; j!=((1ull<<${D})+C-1)/C; j++) a[j]=u64(rand()) << 32 | rand();
    u64 m=(p2+p1)/2;
    n[m][p2]=Ref<${D}>{a, m}[p2];
    b1.set(a, m, p2);
    b2=Bitstring_raw(n.id(), p1, p2);
    eq &= b1.equal(b2, p1, p2);
   
    //set single
    u64 ln[7]={0xffffffffffffffff, 0x5555555555555555, 0x1111111111111111, 0x0101010101010101, 0x0001000100010001, 0x0000000100000001, 0x0000000000000001};
    u64 v=rand()%(1ull<<${D});
    n[p1][p2]=v*ln[posu(${D})];
    for(auto i=p1; i!=p2+1; i++) b1.set(v<<i%C*${D}, i, i);
    b2=Bitstring_raw(n.id(), p1, p2);
    eq &= b1.equal(b2, p1, p2);
   
    //psf 
    auto n2=node::alloc(sz, 0, tree::PSF);
    n2[p1][p2]=Ref<${D}>{a, p1}[p2];
    b1=Bitstring_raw(a, p1, p2);
    b2=Bitstring_raw(n2.id(), p1, p2);
    eq &= b1.equal(b2, p1, p2);

    //inf
    auto n3=node::alloc(sz, 0, tree::INF);
    n3[p1][p2]=Ref<${D}>{a, p1}[p2];
    b1=Bitstring_raw(a, p1, p2);
    b2=Bitstring_raw(n3.id(), p1, p2);
    eq &= b1.equal(b2, p1, p2);

    node::dealloc(n2);
    node::dealloc(n3);
    node::dealloc(n);
  }

  return eq; 
}

int main() {
  srand(s);
  std::cout << "seed: " << s << std::endl;
  bool eq=1;
  eq &= test_powers();
  eq &= test_sz();
  eq &= test_lb();
  eq &= test_bit_operations();
  return eq==true? 0: 1;
}
