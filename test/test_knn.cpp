#include <cstdint>
#include <vector>
#include <functional>
#include "../ph_tree.h"
#include "../bitstring.h"

constexpr u64 D=${D};
constexpr u64 H=${H};

using bstr=Bitstring<D, H>;
using tree=PHTree<D, H>;
using point=PHTree<D, H>::point;

u64 s=time(0);

bool test_rectQuery(int n, int k) {
  PHTree<D, H> t;

  auto generate = []() { bstr s; for(auto j=0; j<H*D; j+=64) s.a[j/64]=u64(rand())<<32 | rand(); return s; };

  std::vector<bstr> gen;
  for(auto i=0; i!=n; i++) gen.push_back(generate());

  for(auto v: gen) t.insert(v);
  gen = std::vector<bstr>(); for(auto it=t.begin(); !it.end(); it++) gen.push_back(*it); //remove duplicates

  auto pivot = generate();

  std::vector<bstr> knn; int i=0; for(auto it=t.knnIterator(pivot); i!=k && !it.end(); it++, i++) knn.push_back(*it); 

  std::sort(gen.begin(), gen.end(), [&](bstr &b1, bstr &b2) { auto p1=t.decode(pivot), p2=t.decode(b1), p3=t.decode(b2); 
      return tree::euclid_dist(p1, p2) < tree::euclid_dist(p1, p3); });

  bool eq=knn.size() == k || knn.size() == gen.size();
  for(auto i=0; i!=knn.size(); i++) { 
    auto p1=t.decode(pivot), p2=t.decode(knn[i]), p3=t.decode(gen[i]);
    eq &= tree::euclid_dist(p1, p2) == tree::euclid_dist(p1, p3);
  }
  return eq? 0: 1;
}

int main() {
  srand(s);
  assertions();
  std::cout << "seed: " << s << std::endl;
  bool val=0;
  val |= test_rectQuery(1e5, 1e5/3);

  return val;
}
