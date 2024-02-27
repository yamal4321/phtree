#include <cstdint>
#include <vector>
#include <functional>
#include "../ph_tree.h"
#include "../bitstring.h"

constexpr u64 D=${D};
constexpr u64 H=${H};

using bstr=Bitstring<D, H>;
using tree=PHTree<D, H>;

u64 s=time(0);

bool inside(u64 v, u64 l, u64 r) { return (v & r | l) == v; }
constexpr u64 pos(u64 v) { return sizeof(unsigned long long)*8 - __builtin_clzll(v) - 1; }
constexpr u64 posu(u64 v) { return pos(2*v-1); }


template <u64 D> struct Node_raw {
  int sz, h;
  Node_raw<D> *a[1ull<<D];

  Node_raw() { sz=h=0; std::fill(a, a+(1ull<<D), nullptr); }
};

template <u64 D, u64 H> struct PHTree_raw {
  ~PHTree_raw() { clean(root); }
  Node_raw<D> *root=0; u64 pz;

  Node_raw<D> *find(bstr b) {
    pz=0;
    for(auto n=root; ; pz++) {
      if(n->a[b[pz]]==nullptr) return n;
      n=n->a[b[pz]];
    }
  }

  void insert(bstr b) {
    if(root==0) { root=new Node_raw<D>(); }
    auto n=find(b);
    for(; pz!=H;) {
      n->a[b[pz]]=new Node_raw<D>();
      n->sz++;
      n=n->a[b[pz]];
      pz++;
    }
  }

  void remove(bstr b) {
    if(root==0) return;
    Node_raw<D> *stack[H+1]; stack[0]=root;
    for(pz=0; pz!=H; pz++) {
      if(stack[pz]->a[b[pz]]==0) { return; }
      stack[pz+1]=stack[pz]->a[b[pz]];
    }
    for(pz=H; pz!=-u64(1); pz--) { if(stack[pz]->sz==0) { delete stack[pz]; if(pz>0) { stack[pz-1]->a[b[pz-1]]=nullptr; stack[pz-1]->sz--; } if(pz==0) root=0; } }
  }

  std::vector<bstr> traverse() {
    std::vector<bstr> ret;
    std::function<void(bstr, u64, Node_raw<D> *)> traverse_impl = [&](bstr str, u64 d, Node_raw<D>* n) {
      if(d==H) { ret.push_back(str); return ; }
      for(u64 i=0ull; i!=(1ull<<D); i++) { if(n->a[i]!=0) { str[d]=i; traverse_impl(str, d+1, n->a[i]); } }
    };
    if(root==0) return ret;
    traverse_impl(bstr(), 0ull, root); return ret;
  }

  void traverse(std::vector<bstr>& v, bstr a, bstr b, Node_raw<D> *n, bstr path=bstr(), u64 am=~0ull, u64 bm=0ull, u64 pz=0) {
    if(pz==H) { v.push_back(path); return; }
    for(u64 i=0; i!=1ull<<D; i++) {
      u64 l=a[pz] & am, r=b[pz] | bm;
      if(n->a[i]==nullptr) continue;
      if(!inside(i, l & ~-(1ull<<D), r & ~-(1ull<<D))) continue;
      path[pz]=i; traverse(v, a, b, n->a[i], path, am & ~((l^r) & i), bm | ((l^r) & ~i), pz+1);
    } 
  }

  void clean(Node_raw<D> *node) {
    if(node==0) return;
    for(u64 i=0; i!=1ull<<D; i++) if(node->a[i]) { clean(node->a[i]); }
    delete node;
  }
};

std::vector<bstr> traverse(PHTree<D, H> &t1) {
  std::vector<bstr> p2;
  for(auto it=t1.begin(); !it.end(); it++) { auto pt=*it; p2.push_back(*(bstr*)&pt); }
  return p2;
}

bool test_rectQuery(int n) {
  PHTree_raw<D, H> t1; PHTree<D, H> t2;
  bstr l, r;
  for(u64 j=0; j<H*D; j+=64) { l.a[j/64]=u64(rand())<<32 | rand(); r.a[j/64]=(u64(rand())<<32 | rand()); } //partial
  for(u64 j=0, m=~-(1ull<<D); j!=H; j++) { u64 a=l[j], b=r[j], bits=m; while(bits) { u64 bit=~(bits-1ull) & bits; if((a&bit) > (b&bit)) { l[j]=u64(l[j]) ^ bit; r[j]=u64(r[j]) ^ bit; m ^= bit; /* std::cout << "bit: " << j << " " << pos(bit) << std::endl; */} bits ^= bit; } }

  std::vector<bstr> gen;
  for(auto i=0; i!=n; i++) { bstr s; for(auto j=0; j<H*D; j+=64) s.a[j/64]=u64(rand())<<32 | rand(); gen.push_back(s); }

  for(auto v: gen) t1.insert(v);
  for(auto v: gen) t2.insert(v);

  std::vector<bstr> found1; t1.traverse(found1, l, r, t1.root);
  std::vector<bstr> found2; for(auto it2=t2.rectIterator(l, r, true); !it2.end(); it2++) found2.push_back(*it2);
 
  std::cout << found1.size() << std::endl << std::flush;
  std::cout << found2.size() << std::endl << std::flush;

  bool query_eq=true;
  if(found1.size()!=found2.size()) query_eq=false; else for(auto i=0; i!=found1.size(); i++) query_eq &= (found1[i] == found2[i]);
  return query_eq? 0: 1;
}

int main() {
  srand(s);
  assertions();
  std::cout << "seed: " << s << std::endl;
  bool val=0;
  val |= test_rectQuery(1e4);
  return val;
}
